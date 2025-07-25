using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class GrassPass : ScriptableRenderPass
{
    // indirect rendering
    private readonly Material GrassMat;
    private ComputeBuffer ArgsBuffer;
    private uint[] Args = new uint[5] { 0, 1, 0, 0, 0 };
    private const int SubMeshIndex = 0;
    private readonly Mesh QuadMesh;

    // height compute
    private ComputeShader HeightCompute;
    private ComputeBuffer PositionAppendBuffer;
    private int MainKernel;
    private Terrain Terrain;

    private readonly RTHandle PixColorHandle;
    private readonly RTHandle PixDepthHandle;
    private readonly RTHandle PixTerrainHandle;

    public GrassPass(RenderTexture RT, ComputeShader HeightCompute, Material GrassMat, Mesh QuadMesh, DownSamplingPass Pass)
    {
        this.Terrain = Terrain.activeTerrain;
        this.HeightCompute = HeightCompute;
        this.GrassMat = GrassMat;
        this.PixColorHandle = Pass.GetColorHandle();
        this.PixDepthHandle = Pass.GetDepthHandle();
        this.PixTerrainHandle = Pass.GetTerrainHandle();
        this.QuadMesh = QuadMesh;
        renderPassEvent = RenderPassEvent.AfterRenderingTransparents;

        InitTerrain();
    }

    public void InitTerrain()
    {
        if (HeightCompute == null || Terrain == null)
            return;

        if (QuadMesh == null || GrassMat == null)
            return;

        // use heightmap to place grass blades in a grid
        var Data = Terrain.terrainData;
        RenderTexture HeightTex = Data.heightmapTexture;
        const float Scale = 1 / 4.0f;
        const float InverseScale = 1 / Scale;
        const int MemSize = sizeof(uint) * 2 + sizeof(float) * (3 + 3 + 3);
        
        MainKernel = HeightCompute.FindKernel("CSMain");
        PositionAppendBuffer = new ComputeBuffer(100000, MemSize, ComputeBufferType.Append);
        PositionAppendBuffer.SetCounterValue(0);
        HeightCompute.SetBuffer(MainKernel, "PositionBuffer", PositionAppendBuffer);
        HeightCompute.SetTexture(MainKernel, "HeightMap", HeightTex);
        HeightCompute.SetFloat("HeightCutoff", 0.004f);
        HeightCompute.SetFloat("InverseScale", InverseScale);
        HeightCompute.SetVector("WorldSize", Data.size);
        HeightCompute.SetVector("TexSize", new(HeightTex.width, 0, HeightTex.height));
        HeightCompute.SetVector("WorldPos", Terrain.transform.position);

        HeightCompute.Dispatch(MainKernel, (int)(HeightTex.width * Scale), (int)(HeightTex.height * Scale), 1);

        // read out the actually found grass positions
        ArgsBuffer = new ComputeBuffer(1, Args.Length * sizeof(uint), ComputeBufferType.IndirectArguments);
        ArgsBuffer.SetData(Args);
        ComputeBuffer.CopyCount(PositionAppendBuffer, ArgsBuffer, 0);
        ArgsBuffer.GetData(Args);
        var TempGrassCount = Args[0];

        // now we can actually fill in the data
        Args[0] = QuadMesh.GetIndexCount(SubMeshIndex);
        Args[1] = TempGrassCount;
        Args[2] = QuadMesh.GetIndexStart(SubMeshIndex);
        Args[3] = QuadMesh.GetBaseVertex(SubMeshIndex);
        ArgsBuffer.SetData(Args);

        GrassMat.SetBuffer("PositionBuffer", PositionAppendBuffer);
        GrassMat.SetTexture("_CopyTex", PixTerrainHandle.rt);
        GrassMat.SetTexture("_TargetTex", PixColorHandle.rt);
    }

    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);
        ConfigureTarget(PixColorHandle, PixDepthHandle);

        ConfigureInput(ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        //ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
    }



    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (PixColorHandle == null)
            return;

        CommandBuffer GrassCmd = CommandBufferPool.Get();
        using (new ProfilingScope(GrassCmd, new ProfilingSampler("GrassPass")))
        {
            Context.ExecuteCommandBuffer(GrassCmd);
            GrassCmd.Clear();

            if (QuadMesh != null && GrassMat != null && ArgsBuffer != null)
            {
                GrassCmd.DrawMeshInstancedIndirect(QuadMesh, SubMeshIndex, GrassMat, 0, ArgsBuffer);
            }
        }
        Context.ExecuteCommandBuffer(GrassCmd);
        GrassCmd.Clear();
        CommandBufferPool.Release(GrassCmd);
    }

    public void Dispose()
    {
        PositionAppendBuffer?.Release();
        ArgsBuffer?.Release();
        PositionAppendBuffer?.Release();
    }
}

