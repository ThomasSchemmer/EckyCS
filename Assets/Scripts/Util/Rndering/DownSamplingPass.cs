using Codice.CM.Client.Differences;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Experimental.Rendering;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;
using static UnityEngine.GraphicsBuffer;

public class DownSamplingPass : ScriptableRenderPass
{
    private RTHandle PixColorHandle;
    private RTHandle PixDepthHandle;

    private List<ShaderTagId> ShaderTags;


    public DownSamplingPass(TTTerrainPass TPass)
    {
        if (TPass == null)
            return;

        renderPassEvent = RenderPassEvent.AfterRenderingTransparents;
        PixColorHandle = TPass.GetColorHandle();
        PixDepthHandle = TPass.GetDepthHandle();

        ShaderTags = new(){
            new ShaderTagId("UniversalForward"),
        };
    }

    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);
        ConfigureTarget(PixColorHandle, PixDepthHandle);

        ConfigureInput(ScriptableRenderPassInput.Normal | ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        //ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
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

            DrawingSettings DrawSettings = CreateDrawingSettings(ShaderTags, ref renderingData, SortingCriteria.CommonOpaque);
        
            FilteringSettings TerrainFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Terrain"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref TerrainFilter);

            if (Game.TryGetService(out ECS ECS) && ECS.TryGetSystems<RenderSystem>(out var SystemList))
            {
                foreach (var System in SystemList)
                {
                    if (System is not RenderSystem RenderSystem)
                        continue;

                    RenderSystem.AddToRenderBuffer(ref Cmd);
                }
            }

            FilteringSettings WorldFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("World"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref WorldFilter);
            
            FilteringSettings WaterFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Water"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref WaterFilter);
        }
        Context.ExecuteCommandBuffer(Cmd);
        Cmd.Clear();
        CommandBufferPool.Release(Cmd);
    }

}
