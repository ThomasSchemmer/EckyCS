using System.Collections;
using System.Collections.Generic;
using Unity.Mathematics;
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

        if (!Game.TryGetService(out PlayerInstantiatorService PlayerService))
            return;

        (Vector3, Vector3) CamPos = PlayerService.GetCamPos();
        Material.SetInt("_ViewState", (int)PlayerService.GetViewState());
        Material.SetVector("_Cam0Pos", CamPos.Item1);
        Material.SetVector("_Cam1Pos", CamPos.Item2);
        Material.SetVector("_ViewAxis", GetViewAxis(CamPos));

        Material.SetTexture("_Player0Tex", TerrainPass.GetColorHandle(0));
        if (PlayerService.GetPlayerCount() == 2)
        {
            Material.SetTexture("_Player1Tex", TerrainPass.GetColorHandle(1));
        }


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

    private Vector2 GetViewAxis((Vector3, Vector3) Cams)
    {
        Vector3 SplitDir = (Cams.Item2 - Cams.Item1).normalized;
        // "right" in worldspace, as displayed on Screen
        Vector2 Right = new Vector2(-0.707f, 0.707f);
        float AngleRad = Mathf.Atan2(SplitDir.z, SplitDir.x) - Mathf.Atan2(Right.y, Right.x);
        AngleRad += AngleRad < 0 ? 2 * Mathf.PI : 0;
        AngleRad -= AngleRad > 2 * Mathf.PI ? 2 * Mathf.PI : 0;
        float c = Mathf.Cos(AngleRad);
        float s = Mathf.Sin(AngleRad);
        Vector2 AxisDir = new(1, 0);
        Vector2 AxisRotated = new(
            AxisDir.x * c - AxisDir.y * s,
            AxisDir.x * s + AxisDir.y * c
        );
        return AxisRotated;
    }
}
