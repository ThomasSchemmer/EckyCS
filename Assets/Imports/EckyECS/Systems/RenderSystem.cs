using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEditor.PackageManager.UI;
using UnityEngine;
using UnityEngine.Profiling;
using static UnityEngine.GraphicsBuffer;

/** 
 * Renders every entity with both SpriteComponent and TransformComponent,
 * by iterating and batching each group. This is obviously slower for a small amount of entities, 
 * so be careful!
 */
public class RenderSystem : MonoBehaviour, ECSSystem
{
    public Material BaseMat;
    public Mesh Mesh;

    private ECS ECS;

    public Dictionary<ComponentGroupIdentifier, RenderInfo> Infos = new();

    public void StartSystem() {

    }

    public void Start()
    {
        Game.RunAfterServiceInit((ECS ECS) =>
        {
            this.ECS = ECS;
            ECS.AddSystem(this);

        });
    }

    private unsafe void Register(ComponentGroupIdentifier Group, void*[] Ptrs, int Count)
    {
        if (Infos.ContainsKey(Group) && Count > Infos[Group].Count)
        {
            Infos[Group].Dispose();
            Infos.Remove(Group);
        }
        if (!Infos.ContainsKey(Group))
        {
            Infos.Add(Group, new());
            Infos[Group].Create(Group, Count, BaseMat, Mesh);
        }

        Infos[Group].Update(Ptrs, Count);
    }

    private void Render()
    {
        Profiler.BeginSample("ECS.RenderSystem.Render");
        foreach (var Pair in Infos)
        {
            var Info = Pair.Value;
            Graphics.RenderMeshIndirect(Info.RP, Mesh, Info.ArgsBuffer, RenderInfo.CommandCount);
        }
        Profiler.EndSample();
    }

    public unsafe void Tick(float Delta)
    {
        if (ECS == null)
            return;

        Profiler.BeginSample("ECS.RenderSystem.Tick");
        ECS.GetProvider().Get<TransformComponent, SpriteComponent>().ForEachGroup((Group, Ptrs, Count) =>
        {
            Register(Group, Ptrs, Count);
        });
        Render();
        Profiler.EndSample();
    }

    public void Destroy()
    {
        foreach (var Tuple in Infos)
        {
            Infos[Tuple.Key].Dispose();
        }
        Infos.Clear();
    }

    public void LateTick(float Delta)
    {
    }

    /** Holds necessary data for rendering a single group */
    public unsafe class RenderInfo
    {
        public GraphicsBuffer PositionBuffer;
        public GraphicsBuffer IDBuffer;
        public GraphicsBuffer ArgsBuffer;
        public IndirectDrawIndexedArgs[] CommandData;
        public const int CommandCount = 1;
        public RenderParams RP;
        public Material Mat;
        public ComponentGroupIdentifier GroupID;
        public Camera Cam;

        public void Dispose()
        {
            PositionBuffer?.Dispose();
            PositionBuffer = null;
            IDBuffer?.Dispose();
            IDBuffer = null;
            ArgsBuffer?.Dispose();
            ArgsBuffer = null;
            Destroy(Mat);
            Mat = null;
        }

        public void Create(ComponentGroupIdentifier GroupID, int Count, Material BaseMat, Mesh Mesh)
        {
            Mat = Instantiate(BaseMat);
            RP = new RenderParams(Mat);
            RP.worldBounds = new Bounds(Vector3.zero, 100 * Vector3.one);
            RP.matProps = new MaterialPropertyBlock();
            Cam = Camera.main;

            ArgsBuffer = new GraphicsBuffer(Target.IndirectArguments, CommandCount, IndirectDrawIndexedArgs.size);
            CommandData = new IndirectDrawIndexedArgs[CommandCount];
            PositionBuffer = new GraphicsBuffer(Target.Structured, Count, GetPositionStride());
            IDBuffer = new GraphicsBuffer(Target.Structured, Count, GetIDStride());

            RP.matProps.SetBuffer("_Positions", PositionBuffer);
            RP.matProps.SetBuffer("_IDs", IDBuffer);
            CommandData[0].indexCountPerInstance = Mesh.GetIndexCount(0);
            this.GroupID = GroupID;
        }

        public void Update(void*[] Ptrs, int Count)
        {
            // would be good to move this into its own functions, but calling generic functions with 
            // ref params and no way to infer their type is.. mid at best. So we dupe the code
            NativeArray<TransformComponent> Transforms = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>(Ptrs[0], Count, Allocator.None);
            NativeArray<EntityID> IDs = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<EntityID>(Ptrs[Ptrs.Length - 1], Count, Allocator.None);

            AtomicSafetyHandle TransformSH = AtomicSafetyHandle.Create();
            AtomicSafetyHandle IDsSH = AtomicSafetyHandle.Create();
            NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Transforms, TransformSH);
            NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref IDs, IDsSH);

            PositionBuffer.SetData(Transforms);
            IDBuffer.SetData(IDs);

            AtomicSafetyHandle.CheckDeallocateAndThrow(TransformSH);
            AtomicSafetyHandle.CheckDeallocateAndThrow(IDsSH);
            AtomicSafetyHandle.Release(TransformSH);
            AtomicSafetyHandle.Release(IDsSH);

            // we don't need to deallocate the NativeArrays as they are repurposed pointers anyway

            CommandData[0].instanceCount = (uint)Count;
            ArgsBuffer.SetData(CommandData);
            RP.matProps.SetFloat("_Count", Count);
            Vector3 CamForward = Vector3.Normalize(Vector3.Cross(new(0, 1, 0), Cam.transform.forward));
            RP.matProps.SetVector("_CamForward", CamForward);
            RP.matProps.SetVector("_CamUp", Cam.transform.up);
            RP.matProps.SetVector("_CamPos", Cam.transform.position);
        }

        private void FillBuffer(void* Ptr, ref GraphicsBuffer Buffer, Type Type)
        {
            // we just want to call NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<T>, but calling generic methods 
            // indirectly is a bit convoluted if they can't infer the type :/

            MethodInfo Method = typeof(NativeArrayUnsafeUtility).GetMethod(nameof(NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray));
            MethodInfo GenericMethod = Method.MakeGenericMethod(Type);
            object BoxedPointer = Pointer.Box(Ptr, typeof(void*));
            Array Array = (Array)GenericMethod.Invoke(null, new object[]
            {
                BoxedPointer, Count, Allocator.None
            });

            // now NativeArray<T> also requires a safety handle, which type - again - cannot be inferred
            
        }

        public int Count
        {
            get { return PositionBuffer.count; }
        }

        private static int GetPositionStride()
        {
            return sizeof(float) * 3;
        }
        private static int GetIDStride()
        {
            return sizeof(uint);
        }
    }
}
