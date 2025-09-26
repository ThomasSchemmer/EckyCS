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
    private List<ShaderTagId> ShaderTags;

    private TTTerrainPass TPass;


    public DownSamplingPass(TTTerrainPass TPass)
    {
        if (TPass == null)
            return;

        renderPassEvent = RenderPassEvent.AfterRenderingTransparents;
        this.TPass = TPass;

        ShaderTags = new(){
            new ShaderTagId("UniversalForward"),
        };
    }

    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);

        ConfigureInput(ScriptableRenderPassInput.Normal | ScriptableRenderPassInput.Depth | ScriptableRenderPassInput.Color);
        //ConfigureClear(ClearFlag.Color, new(1, 0, 0, 1));
    }


    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (TPass == null)
            return;

        int i = TPass.GetPlayerIndex(ref renderingData);
        CommandBuffer Cmd = CommandBufferPool.Get();
        using (new ProfilingScope(Cmd, new ProfilingSampler("DownSamplingPass "+i)))
        {
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();
            Cmd.SetRenderTarget(
                TPass.GetColorHandle(i),
                TPass.GetDepthHandle(i)
            );

            DrawingSettings DrawSettings = CreateDrawingSettings(ShaderTags, ref renderingData, SortingCriteria.CommonOpaque);
        
            FilteringSettings TerrainFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Terrain"));
            Context.DrawRenderers(renderingData.cullResults, ref DrawSettings, ref TerrainFilter);

            if (Game.TryGetService(out EckyCS ECS) && ECS.TryGetSystems<RenderSystem>(out var SystemList))
            {
                foreach (var System in SystemList)
                {
                    if (System is not RenderSystem RenderSystem)
                        continue;

                    RenderSystem.AddToRenderBuffer(ref Cmd);
                }
            }

            Cmd.SetRenderTarget(
                TPass.GetColorHandle(i),
                TPass.GetDepthHandle(i)
            );
            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();

            // TODO: custom shadow map to discard unity's standard renderer
            if (!renderingData.cameraData.camera.TryGetCullingParameters(out var Params))
                return;

            Params.cullingMask |= 1u << LayerMask.NameToLayer("Default");
            Params.cullingMask |= 1u << LayerMask.NameToLayer("World");
            Params.cullingMask |= 1u << LayerMask.NameToLayer("Water");
            Params.cullingMask |= 1u << LayerMask.NameToLayer("Ability");
            var CullResults = Context.Cull(ref Params);

            FilteringSettings WorldFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("World"));
            Context.DrawRenderers(CullResults, ref DrawSettings, ref WorldFilter);

            FilteringSettings WaterFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Water"));
            Context.DrawRenderers(CullResults, ref DrawSettings, ref WaterFilter);

            FilteringSettings AbilityFilter = new FilteringSettings(layerMask: 1 << LayerMask.NameToLayer("Ability"));
            Context.DrawRenderers(CullResults, ref DrawSettings, ref AbilityFilter);
        }
        Context.ExecuteCommandBuffer(Cmd);
        Cmd.Clear();
        CommandBufferPool.Release(Cmd);
    }

}
