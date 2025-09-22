using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class EdgeHighlightPass : ScriptableRenderPass
{
    public Material Material;
    public TTTerrainPass TerrainPass;


    public EdgeHighlightPass(Material Material, TTTerrainPass TerrainPass)
    {
        this.Material = Material;
        this.TerrainPass = TerrainPass;
        renderPassEvent = RenderPassEvent.AfterRenderingPostProcessing;

    }

    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (Material == null || TerrainPass == null)
            return;

        int i = TerrainPass.GetPlayerIndex(ref renderingData);
        Material.SetTexture("_MainTex", TerrainPass.GetColorHandle(i));

        CommandBuffer Cmd = CommandBufferPool.Get();
        using (new ProfilingScope(Cmd, new ProfilingSampler("EdgeHighlight Pass")))
        {

            Context.ExecuteCommandBuffer(Cmd);
            Cmd.Clear();
            Cmd.DrawProcedural(Matrix4x4.identity, Material, 0, MeshTopology.Triangles, 3, 1);

        }
        Context.ExecuteCommandBuffer(Cmd);
        Cmd.Clear();
        CommandBufferPool.Release(Cmd);
    }


    public override void Configure(CommandBuffer cmd, RenderTextureDescriptor cameraTextureDescriptor)
    {
        base.Configure(cmd, cameraTextureDescriptor);
        ConfigureInput(ScriptableRenderPassInput.Normal | ScriptableRenderPassInput.Depth);
    }
}
