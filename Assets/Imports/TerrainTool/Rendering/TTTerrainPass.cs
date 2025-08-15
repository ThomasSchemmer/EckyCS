using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class TTTerrainPass : ScriptableRenderPass
{

    private readonly RTHandle PixColorHandle;
    private readonly RTHandle PixDepthHandle;
    private readonly RTHandle PixTerrainColorHandle;
    private const int Height = 512, Width = (int)(Height * 1.7777f);

    public readonly TTManager Manager;

    readonly static FieldInfo depthTextureFieldInfo = typeof(UniversalRenderer).GetField("m_DepthTexture", BindingFlags.NonPublic | BindingFlags.Instance);
    RTHandle CamDepthHandle;

    public TTTerrainPass()
    {
        renderPassEvent = RenderPassEvent.BeforeRenderingOpaques;

        // todo: stop flickering of color info
        PixColorHandle = RTHandles.Alloc(width: Width, height: Height, filterMode: FilterMode.Point);
        PixDepthHandle = RTHandles.Alloc(width: Width, height: Height, depthBufferBits: DepthBits.Depth16, filterMode: FilterMode.Point);
        PixTerrainColorHandle = RTHandles.Alloc(width: PixColorHandle.rt.width / 2, height: PixColorHandle.rt.height / 2, filterMode: FilterMode.Point);

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
        ConfigureTarget(PixColorHandle, PixDepthHandle);

        ConfigureInput(ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
    }

    public void SetHandle(UniversalRenderer UR)
    {
        // can't get actual depth texture directly, so use reflection
        CamDepthHandle = depthTextureFieldInfo.GetValue(UR) as RTHandle;
    }


    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (PixColorHandle == null || Manager == null)
            return;

        CommandBuffer Cmd = CommandBufferPool.Get();
        using (new ProfilingScope(Cmd, new ProfilingSampler("TerrainPass")))
        {
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

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
            Cmd.Blit(PixColorHandle, PixTerrainColorHandle);
            // fun fact: blit changes the render target -.-
            Cmd.SetRenderTarget(PixColorHandle, PixDepthHandle);
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();
        }
        Context.ExecuteCommandBuffer(Cmd);
        Cmd.Clear();
        CommandBufferPool.Release(Cmd);
    }

    public void Dispose()
    {
        PixColorHandle?.Release();
        PixDepthHandle?.Release();
        PixTerrainColorHandle?.Release();
    }


    public RTHandle GetTerrainHandle()
    {
        return PixTerrainColorHandle;
    }

    public RTHandle GetColorHandle()
    {
        return PixColorHandle;
    }

    public RTHandle GetDepthHandle()
    {
        return PixDepthHandle;
    }
}
