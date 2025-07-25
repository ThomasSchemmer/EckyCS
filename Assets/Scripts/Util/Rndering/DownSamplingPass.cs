using Codice.CM.Client.Differences;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Experimental.Rendering;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class DownSamplingPass : ScriptableRenderPass
{
    private RTHandle PixColorHandle;
    private RTHandle PixDepthHandle;
    private RTHandle PixTerrainHandle;

    private const int Height = 512, Width = (int)(Height * 1.7777f);

    private List<ShaderTagId> ShaderTags;


    public DownSamplingPass()
    {
        renderPassEvent = RenderPassEvent.AfterRenderingTransparents;
        PixColorHandle = RTHandles.Alloc(width: Width, height: Height);
        PixDepthHandle = RTHandles.Alloc(width: Width, height: Height, depthBufferBits: DepthBits.Depth16);
        PixTerrainHandle = RTHandles.Alloc(width: Width / 2, height: Height / 2);

        ShaderTags = new(){
            new ShaderTagId("UniversalForward"),
        };
    }

    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);
        ConfigureTarget(PixColorHandle, PixDepthHandle);

        ConfigureInput(ScriptableRenderPassInput.Normal | ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
    }


    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (PixColorHandle == null)
            return;

        CommandBuffer Cmd = CommandBufferPool.Get();
        using (new ProfilingScope(Cmd, new ProfilingSampler("DownSamplingPass")))
        {
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

            Cmd.ClearRenderTarget(true, false, new(0, 0, 0, 0));
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

            DrawingSettings DrawSettings = CreateDrawingSettings(ShaderTags, ref renderingData, SortingCriteria.CommonOpaque);
        
            FilteringSettings TerrainFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Terrain"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref TerrainFilter);

            Cmd.Blit(PixColorHandle, PixTerrainHandle);
            Cmd.SetRenderTarget(PixColorHandle, PixDepthHandle);
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

            FilteringSettings WorldFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("World"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref WorldFilter);
            
            FilteringSettings WaterFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Water"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref WaterFilter);
        }
        Context.ExecuteCommandBuffer(Cmd);
        Cmd.Clear();
        CommandBufferPool.Release(Cmd);
    }

    public void Dispose()
    {
        PixColorHandle?.Release();
        PixDepthHandle?.Release();
        PixTerrainHandle?.Release();
    }

    public RTHandle GetColorHandle()
    {
        return PixColorHandle;
    }

    public RTHandle GetDepthHandle()
    {
        return PixDepthHandle;
    }

    public RTHandle GetTerrainHandle()
    {
        return PixTerrainHandle;
    }

    // DO NOT DELETE _ FALLBACK FOR IMG COPY
    //Context.ExecuteCommandBuffer(Cmd);
    //Cmd.Clear();
    //Cmd.Blit(renderingData.cameraData.renderer.cameraColorTargetHandle, PixelizeHandle);
}
