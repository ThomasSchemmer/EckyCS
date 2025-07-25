using System.Collections;
using System.Collections.Generic;
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
        if (renderingData.cameraData.isSceneViewCamera || renderingData.cameraData.isPreviewCamera)
            return;

        Renderer.EnqueuePass(DSPass);
        if (bEnableGrass)
        {
            Renderer.EnqueuePass(GrassPass);
        }
        if (bEnableEdge)
        {
            Renderer.EnqueuePass(EdgePass);
        }
    }

    public override void Create()
    {
        DSPass = new DownSamplingPass();
        GrassPass = new GrassPass(
            RT, 
            HeightCompute, 
            GrassMat, 
            QuadMesh, 
            DSPass
        );
        EdgePass = new EdgeHighlightPass(EdgeMaterial, DSPass.GetColorHandle());
    }

    protected override void Dispose(bool disposing)
    {
        DSPass.Dispose();
        GrassPass.Dispose();
    }

}
