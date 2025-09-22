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
    private TTTerrainPass TerrainPass;

    public GrassPass(ComputeShader HeightCompute, Material GrassMat, Mesh QuadMesh, TTTerrainPass TerrainPass)
    {
        if (TerrainPass == null)
            return;

        this.HeightCompute = HeightCompute;
        this.GrassMat = GrassMat;
        this.TerrainManager = TerrainPass.Manager;
        this.QuadMesh = QuadMesh;
        this.TerrainManager = TerrainPass.Manager;
        this.TerrainPass = TerrainPass;

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
        const int MemSize = sizeof(float) * 3;
        
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

        // now we can actually fill in the data
        Args[0] = QuadMesh.GetIndexCount(SubMeshIndex);
        Args[1] = 0;    // to be filled by compute
        Args[2] = QuadMesh.GetIndexStart(SubMeshIndex);
        Args[3] = QuadMesh.GetBaseVertex(SubMeshIndex);
        ArgsBuffer = new ComputeBuffer(1, Args.Length * sizeof(uint), ComputeBufferType.IndirectArguments);
        ArgsBuffer.SetData(Args);
        ComputeBuffer.CopyCount(PositionAppendBuffer, ArgsBuffer, sizeof(uint));

        GrassMat.SetBuffer("PositionBuffer", PositionAppendBuffer);
    }

    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);

        ConfigureInput(ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        //ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
    }



    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (TerrainPass == null)
            return;

        int i = TerrainPass.GetPlayerIndex(ref renderingData);
        GrassMat.SetVector(
            "_CamOffset",
            renderingData.cameraData.camera.transform.position
        );
        GrassMat.SetTexture("_CopyTex", TerrainPass.GetTerrainHandle(i).rt);
        GrassMat.SetTexture("_TargetTex", TerrainPass.GetColorHandle(i).rt);

        CommandBuffer GrassCmd = CommandBufferPool.Get();
        using (new ProfilingScope(GrassCmd, new ProfilingSampler("GrassPass")))
        {
            Context.ExecuteCommandBuffer(GrassCmd);
            GrassCmd.Clear();
            GrassCmd.SetRenderTarget(
                TerrainPass.GetColorHandle(i),
                TerrainPass.GetDepthHandle(i)
            );

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

