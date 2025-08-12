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
    private TTManager TerrainManager;
    private Camera Cam;

    private readonly RTHandle PixColorHandle;
    private readonly RTHandle PixDepthHandle;
    private readonly RTHandle PixTerrainHandle;

    public GrassPass(ComputeShader HeightCompute, Material GrassMat, Mesh QuadMesh, TTTerrainPass TerrainPass)
    {
        if (TerrainPass == null)
            return;

        this.HeightCompute = HeightCompute;
        this.GrassMat = GrassMat;
        this.PixColorHandle = TerrainPass.GetColorHandle();
        this.PixDepthHandle = TerrainPass.GetDepthHandle();
        this.PixTerrainHandle = TerrainPass.GetTerrainHandle();
        this.TerrainManager = TerrainPass.Manager;
        this.QuadMesh = QuadMesh;
        this.TerrainManager = TerrainPass.Manager;
        this.Cam = Camera.main;

        renderPassEvent = RenderPassEvent.AfterRenderingTransparents;

        InitTerrain();
    }

    public void InitTerrain()
    {
        if (HeightCompute == null || TerrainManager == null)
            return;

        if (QuadMesh == null || GrassMat == null)
            return;

        // use heightmap to place grass blades in a grid
        RenderTexture HeightTex = TerrainManager.HeightRT;
        const float Scale = .75f;
        const float InverseScale = 1 / Scale;
        const int MemSize = sizeof(uint) * 2 + sizeof(float) * (3 + 3 + 3);
        
        MainKernel = HeightCompute.FindKernel("CSMain");
        PositionAppendBuffer = new ComputeBuffer(100000, MemSize, ComputeBufferType.Append);
        PositionAppendBuffer.SetCounterValue(0);
        HeightCompute.SetBuffer(MainKernel, "PositionBuffer", PositionAppendBuffer);
        HeightCompute.SetTexture(MainKernel, "HeightMap", HeightTex);
        HeightCompute.SetInt("Bands", TerrainManager.Settings.Bands);
        HeightCompute.SetFloat("HeightCutoff", 0.004f);
        HeightCompute.SetFloat("InverseScale", InverseScale);
        HeightCompute.SetVector("WorldSize", (Vector3)TerrainManager.Settings.WorldSize);
        HeightCompute.SetVector("TexSize", (Vector2)TerrainManager.Settings.TexSize);
        HeightCompute.SetVector("WorldPos", TerrainManager.transform.position);

        HeightCompute.Dispatch(MainKernel, Mathf.RoundToInt(HeightTex.width * Scale), Mathf.RoundToInt(HeightTex.height * Scale), 1);

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

        GrassMat.SetVector("_CamOffset", Cam.transform.position);
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

