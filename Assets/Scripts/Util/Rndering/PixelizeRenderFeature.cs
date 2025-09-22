using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Experimental.Rendering;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

/**
 * Reimplementation of Unity's FSRP, as the overlay camera doesn't work with it
 * Stripped out some unnecessary code as well
 */
public class PixelizeRenderFeature : ScriptableRendererFeature
{
    public UniversalRendererData URPRenderer;
    public float Cutoff;
    public RenderTexture RT;
    public bool bEnableGrass;
    public bool bEnableEdge;
    public Material GrassMat;
    public Material EdgeMaterial;
    public Mesh QuadMesh;
    public ComputeShader HeightCompute;

    private DownSamplingPass DSPass;
    private EdgeHighlightPass EdgePass;
    private GrassPass GrassPass;

    public override void AddRenderPasses(ScriptableRenderer Renderer, ref RenderingData renderingData)
    {
        if (renderingData.cameraData.isPreviewCamera)
            return;

        Renderer.EnqueuePass(DSPass);
        if (bEnableGrass)
        {
            Renderer.EnqueuePass(GrassPass);
        }

        if (renderingData.cameraData.isSceneViewCamera)
            return;

        if (bEnableEdge)
        {
            Renderer.EnqueuePass(EdgePass);
        }   
    }

    public override void Create()
    {
        TerrainRenderFeature TRF = URPRenderer.rendererFeatures.Where(x => x is TerrainRenderFeature).FirstOrDefault() as TerrainRenderFeature;
        if (!TRF ||TRF.TerrainPass == null)
            return;

        TTTerrainPass TPass = TRF.TerrainPass;
        DSPass = new DownSamplingPass(TPass);
        GrassPass = new GrassPass(
            HeightCompute, 
            GrassMat, 
            QuadMesh,
            TPass
        );
        EdgePass = new EdgeHighlightPass(EdgeMaterial, TPass);
    }

    protected override void Dispose(bool disposing)
    {
        GrassPass.Dispose();
    }

}
