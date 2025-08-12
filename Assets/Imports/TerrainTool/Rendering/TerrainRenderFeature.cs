using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering.Universal;

public class TerrainRenderFeature : ScriptableRendererFeature
{
    public TTTerrainPass TerrainPass;

    public override void AddRenderPasses(ScriptableRenderer Renderer, ref RenderingData renderingData)
    {
        if (renderingData.cameraData.isPreviewCamera || renderingData.cameraData.isSceneViewCamera)
            return;

        Renderer.EnqueuePass(TerrainPass);
    }

    public override void Create()
    {
        TerrainPass = new TTTerrainPass();
    }

    public override void SetupRenderPasses(ScriptableRenderer renderer, in RenderingData renderingData)
    {
        base.SetupRenderPasses(renderer, renderingData);
        if (renderer is UniversalRenderer UR)
        {
            TerrainPass.SetHandle(UR);
        }
    } 

    protected override void Dispose(bool disposing)
    {
        TerrainPass.Dispose();
    }

}
