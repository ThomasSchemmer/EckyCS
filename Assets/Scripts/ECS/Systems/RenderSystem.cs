using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;
using UnityEngine.Profiling;
using static UnityEngine.GraphicsBuffer;

/** 
 * Renders every entity with both SpriteComponent and TransformComponent,
 * by iterating and batching each group
 */
public class RenderSystem : MonoBehaviour, ECSSystem
{
    public Material BaseMat;
    public Mesh Mesh;

    private ECS ECS;

    private readonly Dictionary<ComponentGroupIdentifier, RenderInfo> Infos = new();

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

    private unsafe void Register(ComponentGroupIdentifier Group, byte*[] Ptrs, int Count)
    {
        int Target = Group.GetSelfIndexOf(typeof(TransformComponent));
        if (Target < 0)
            return;

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
            Graphics.RenderMeshIndirect(Info.RP, Mesh, Info.CommandBuffer, RenderInfo.CommandCount);
        }
        Profiler.EndSample();
    }

    public unsafe void Tick(float Delta)
    {
        if (ECS == null)
            return;

        Profiler.BeginSample("ECS.RenderSystem.Tick");
        ECS.Get<TransformComponent, SpriteComponent>().ForEachGroup((Group, Ptrs, Count) =>
        {
            Register(Group, Ptrs, Count);
        });
        Render();
        Profiler.EndSample();
    }

    public void Destroy()
    {
    }

    public void LateTick(float Delta)
    {
    }

    /** Holds necessary data for rendering a single group */
    private unsafe class RenderInfo
    {
        public GraphicsBuffer PositionBuffer;
        public GraphicsBuffer CommandBuffer;
        public IndirectDrawIndexedArgs[] CommandData;
        public const int CommandCount = 1;
        public RenderParams RP;
        public Material Mat;
        public ComponentGroupIdentifier GroupID;

        public void Dispose()
        {
            PositionBuffer?.Dispose();
            PositionBuffer = null;
            CommandBuffer?.Dispose();
            CommandBuffer = null;
            Destroy(Mat);
            Mat = null;
        }

        public void Create(ComponentGroupIdentifier GroupID, int Count, Material BaseMat, Mesh Mesh)
        {
            Mat = Instantiate(BaseMat);
            RP = new RenderParams(Mat);
            RP.worldBounds = new Bounds(Vector3.zero, 100 * Vector3.one);
            RP.matProps = new MaterialPropertyBlock();

            CommandBuffer = new GraphicsBuffer(Target.IndirectArguments, CommandCount, IndirectDrawIndexedArgs.size);
            CommandData = new IndirectDrawIndexedArgs[CommandCount];
            PositionBuffer = new GraphicsBuffer(Target.Structured, Count, GetStride());

            RP.matProps.SetBuffer("_Positions", PositionBuffer);
            CommandData[0].indexCountPerInstance = Mesh.GetIndexCount(0);
            this.GroupID = GroupID;
        }

        public void Update(byte*[] Ptrs, int Count)
        {
            int Target = GroupID.GetSelfIndexOf(typeof(TransformComponent));
            if (Target < 0)
                return;

            var Array = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>(Ptrs[Target], Count, Allocator.None);
            AtomicSafetyHandle AtomicSafetyHandle = AtomicSafetyHandle.Create();
            NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Array, AtomicSafetyHandle);

            PositionBuffer.SetData(
                Array
            );

            AtomicSafetyHandle.CheckDeallocateAndThrow(AtomicSafetyHandle);
            AtomicSafetyHandle.Release(AtomicSafetyHandle);

            CommandData[0].instanceCount = (uint)Count;
            CommandBuffer.SetData(CommandData);
            RP.matProps.SetFloat("_Count", Count);
        }

        public int Count
        {
            get { return PositionBuffer.count; }
        }

        private static int GetStride()
        {
            return sizeof(float) * 3;
        }
    }
}
