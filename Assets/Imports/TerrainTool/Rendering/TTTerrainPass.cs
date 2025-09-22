using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class TTTerrainPass : ScriptableRenderPass
{

    private readonly RTHandle[] PixColorHandles;
    private readonly RTHandle[] PixDepthHandles;
    private readonly RTHandle[] PixTerrainColorHandles;
    private const int Width = 1920, Height = 1080;

    public readonly TTManager Manager;

    private readonly static FieldInfo depthTextureFieldInfo = typeof(UniversalRenderer).GetField("m_DepthTexture", BindingFlags.NonPublic | BindingFlags.Instance);
    private RTHandle CamDepthHandle;

    public TTTerrainPass()
    {
        renderPassEvent = RenderPassEvent.BeforeRenderingOpaques;

        // todo: stop flickering of color info
        int Count = Game.TargetPlayerCount;
        PixColorHandles = new RTHandle[Count];
        PixDepthHandles = new RTHandle[Count];
        PixTerrainColorHandles = new RTHandle[Count];
        for (int i = 0; i < Count; i++) {
            PixColorHandles[i] = RTHandles.Alloc(width: Width, height: Height, filterMode: FilterMode.Trilinear);
            PixDepthHandles[i] = RTHandles.Alloc(width: Width, height: Height, depthBufferBits: DepthBits.Depth16, filterMode: FilterMode.Point);
            PixTerrainColorHandles[i] = RTHandles.Alloc(width: PixColorHandles[i].rt.width / 2, height: PixColorHandles[i].rt.height / 2, filterMode: FilterMode.Point);

        }

        GameObject Terrain = GameObject.Find("Terrain");
        if (!Terrain)
            return;

        Manager = Terrain.GetComponent<TTManager>();
        if (!Manager.IsInit())
        {
            Manager.Init();
            Manager.GenerateMesh();
        }

    }

    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);

        ConfigureInput(ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
    }

    public void SetHandle(UniversalRenderer UR)
    {
        // can't get actual depth texture directly, so use reflection
        CamDepthHandle = depthTextureFieldInfo.GetValue(UR) as RTHandle;
    }

    public int GetPlayerIndex(ref RenderingData renderingData)
    {
        if (renderingData.cameraData.isPreviewCamera || renderingData.cameraData.isSceneViewCamera)
            return 0;

        return renderingData.cameraData.camera.CompareTag("MainCamera") ? 0 : 1;
    }

    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (PixColorHandles == null || Manager == null)
            return;

        int i = GetPlayerIndex(ref renderingData);
        CommandBuffer Cmd = CommandBufferPool.Get();
        using (new ProfilingScope(Cmd, new ProfilingSampler("TerrainPass")))
        {
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

            Cmd.SetRenderTarget(
                PixColorHandles[i],
                PixDepthHandles[i]
            );
            Cmd.ClearRenderTarget(true, false, new(0, 0, 0, 0));
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

            // regular color
            Cmd.DrawProceduralIndirect(
                Matrix4x4.identity,
                Manager.TerrainMat, 
                0,
                MeshTopology.Triangles,
                Manager.RenderArgsBuffer
            );
            
            // depth pass for terrain
            if (CamDepthHandle != null) {
                Cmd.SetRenderTarget(
                    renderingData.cameraData.renderer.cameraColorTargetHandle,
                    CamDepthHandle
                );
                Cmd.DrawProceduralIndirect(
                    Matrix4x4.identity,
                    Manager.TerrainMat,
                    1,
                    MeshTopology.Triangles,
                    Manager.RenderArgsBuffer
                );
            }

            // use a low res texture for sampling grass color
            // which is only influenced by the underlying terrain, so we can copy the current color tex 
            Cmd.Blit(
                PixColorHandles[i],
                PixTerrainColorHandles[i]
            );
            // fun fact: blit changes the render target -.-
            Cmd.SetRenderTarget(
                PixColorHandles[i], 
                PixDepthHandles[i]
            );
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();
        }
        Context.ExecuteCommandBuffer(Cmd);
        Cmd.Clear();
        CommandBufferPool.Release(Cmd);
    }

    public void Dispose()
    {
        for (int i = 0; i < Game.TargetPlayerCount; i++)
        {
            PixColorHandles[i]?.Release();
            PixDepthHandles[i]?.Release();
            PixTerrainColorHandles[i]?.Release();
        }
    }


    public RTHandle GetTerrainHandle(int i)
    {
        return PixTerrainColorHandles[i];
    }

    public RTHandle GetColorHandle(int i)
    {
        return PixColorHandles[i];
    }

    public RTHandle GetDepthHandle(int i)
    {
        return PixDepthHandles[i];
    }
}
