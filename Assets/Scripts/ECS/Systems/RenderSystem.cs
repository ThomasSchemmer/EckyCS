using System;
using UnityEngine;
using UnityEngine.Profiling;
using UnityEngine.UIElements;

public class RenderSystem : MonoBehaviour, ECSSystem
{
    public Material Mat;
    public Mesh Mesh;

    private ECS ECS;

    private GraphicsBuffer CommandBuffer, MeshPositions;
    private GraphicsBuffer.IndirectDrawIndexedArgs[] CommandData;
    private const int CommandCount = 1;
    private Vector3[] Positions;
    private RenderParams RP;

    public void StartSystem() {

    }

    public void Start()
    {
        Game.RunAfterServiceInit((ECS ECS) =>
        {
            this.ECS = ECS;
            ECS.AddSystem(this);

            Positions = new Vector3[ECS.MaxEntities + 300];
            MeshPositions = new GraphicsBuffer(GraphicsBuffer.Target.Structured, ECS.MaxEntities +300, GetStride());

            CommandBuffer = new GraphicsBuffer(GraphicsBuffer.Target.IndirectArguments, CommandCount, GraphicsBuffer.IndirectDrawIndexedArgs.size);
            CommandData = new GraphicsBuffer.IndirectDrawIndexedArgs[CommandCount];

            RP = new RenderParams(Mat);
            RP.worldBounds = new Bounds(Vector3.zero, 100 * Vector3.one);
            RP.matProps = new MaterialPropertyBlock();
            RP.matProps.SetBuffer("_Positions", MeshPositions);
        });
    }

    public unsafe void Tick(float Delta)
    {
        if (ECS == null)
            return;

        Profiler.BeginSample("ECS.RenderSystem.Tick");
        ECS.Get<TransformComponent>().ForEachGroup((Group, Ptrs, Count) =>
        {
            Profiler.BeginSample("ECS.RenderSystem.Tick.CopyData");
            Profiler.BeginSample("ECS.RenderSystem.Tick.CopyData.CPU");
            fixed (Vector3* Pos = &Positions[0]) {
                Buffer.MemoryCopy(Ptrs[0], Pos, Positions.Length * GetStride(), Count * GetStride());
            }
            Profiler.EndSample();
            Profiler.BeginSample("ECS.RenderSystem.Tick.CopyData.GPU");
            MeshPositions.SetData(Positions);
            Profiler.EndSample();
            Profiler.EndSample();
            Profiler.BeginSample("ECS.RenderSystem.Tick.RenderFrame");
            CommandData[0].indexCountPerInstance = Mesh.GetIndexCount(0);
            CommandData[0].instanceCount = 10;
            CommandBuffer.SetData(CommandData);
            RP.matProps.SetFloat("_Count", Count);
            Graphics.RenderMeshIndirect(RP, Mesh, CommandBuffer, CommandCount);
            Profiler.EndSample();
        });
        Profiler.EndSample();
    }

    private int GetStride()
    {
        return sizeof(float) * 3;
    }

    public void Destroy()
    {
        CommandBuffer?.Release();
        CommandBuffer = null;
        MeshPositions?.Release();
        MeshPositions = null;
    }

    public void LateTick(float Delta)
    {
    }
}
