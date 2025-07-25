using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

public class EdgeHighlightPass : ScriptableRenderPass
{
    public Material Material;
    public RTHandle Handle;


    public EdgeHighlightPass(Material Material, RTHandle Handle)
    {
        this.Material = Material;
        this.Handle = Handle;
        renderPassEvent = RenderPassEvent.AfterRenderingPostProcessing;

        if (Material != null && Handle != null)
        {
            Material.SetTexture("_MainTex", Handle);
        }
    }

    public override void Execute(ScriptableRenderContext Context, ref RenderingData renderingData)
    {
        if (Material == null)
            return;

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
