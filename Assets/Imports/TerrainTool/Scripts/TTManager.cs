using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.UIElements;

[ExecuteInEditMode]
[Serializable]
public class TTManager : MonoBehaviour
{
    public TTSettings Settings;

    public ComputeShader Shader;
    public Material TerrainMat;

    private ComputeBuffer PositionBuffer;
    private ComputeBuffer TriangleAppendBuffer;
    private uint[] Args = new uint[5] { 0, 1, 0, 0, 0 };
    public ComputeBuffer RenderArgsBuffer;
    private int GenerateBaseKernel, GenerateHeightKernel, PaintKernel, ResetKernel, PixelKernel;
    private readonly int VertexCount = 6;

    public RenderTexture HeightRT;
    private RTHandle HeightRTHandle;
    public RenderTexture PreviewRT;
    private RTHandle PreviewRTHandle;

    private bool bHasPreview;
    private const String HeightPath = "/Textures/Heightmap.png";

    public void OnRenderObject()
    {
        if (PositionBuffer == null || RenderArgsBuffer == null || TerrainMat == null)
            return;

        bool SceneViewHasFocus = SceneView.sceneViews.Cast<SceneView>().Any(window => window.hasFocus);
        if (!SceneViewHasFocus)
            return;

        TerrainMat.SetPass(0);
        Graphics.DrawProceduralIndirectNow(MeshTopology.Triangles, RenderArgsBuffer);
    }

    public void Init()
    {
        if (Settings == null || Settings.TexSize.magnitude == 0)
            return;

        HeightRTHandle = RTHandles.Alloc(width: Settings.TexSize.x, height: Settings.TexSize.y, enableRandomWrite: true, useMipMap: false);
        HeightRT = HeightRTHandle.rt;
        PreviewRTHandle = RTHandles.Alloc(width: Settings.TexSize.x, height: Settings.TexSize.y, enableRandomWrite: true, useMipMap: false);
        PreviewRT = PreviewRTHandle.rt;

        int Count = GetCount();
        PositionBuffer = new(Count, sizeof(float) * 3);

        RenderArgsBuffer = new ComputeBuffer(4, sizeof(int), ComputeBufferType.IndirectArguments);
        RenderArgsBuffer.SetData(new int[] { Count, 1, 0, 0 });

        TriangleAppendBuffer = new ComputeBuffer(100000, sizeof(float) * 3 * 3, ComputeBufferType.Append);

        if (Shader == null)
            return;

        GenerateBaseKernel = Shader.FindKernel("GenerateBaseMesh");
        GenerateHeightKernel = Shader.FindKernel("GenerateHeightMesh");
        PaintKernel = Shader.FindKernel("Paint");
        ResetKernel = Shader.FindKernel("Reset");
        PixelKernel = Shader.FindKernel("Pixel");
    }

    public void GenerateDefaultMesh()
    {
        if (HeightRTHandle == null)
        {
            Init();
        }
        if (Shader == null || HeightRTHandle == null)
            return;

        Shader.SetVector("_TexSize", (Vector2)Settings.TexSize);
        Shader.SetVector("_WorldSize", (Vector3)Settings.WorldSize);
        Shader.SetVector("_WorldPos", transform.position);
        Shader.SetTexture(GenerateBaseKernel, "Result", HeightRTHandle.rt);
        Shader.SetBuffer(GenerateBaseKernel, "PositionBuffer", PositionBuffer);
        Shader.Dispatch(GenerateBaseKernel, HeightRT.width - 1, HeightRT.height - 1, 1);

        TerrainMat.SetTexture("_HeightTex", HeightRTHandle.rt);
        TerrainMat.SetBuffer("PositionBuffer", PositionBuffer);
        TerrainMat.SetFloat("_Width", Settings.TexSize.x);
        TerrainMat.SetVector("_WorldPos", transform.position);
        ResetRT(true);
        ResetRT(false);
    }

    public void OnSceneGUI(SceneView View, TTBrushSettings Brush)
    {
        TerrainMat.SetVector("_MousePosition", ProjectMouseOnPlane(View));
        TerrainMat.SetFloat("_BrushSize", Brush.Size);
    }

    public void Brush(TTBrushSettings Brush)
    {
        if (Brush.CurrentWorldPos.w == 0)
            return;

        Rect Container = new (
            new Vector2(transform.position.x, transform.position.z), 
            new Vector2(Settings.WorldSize.x, Settings.WorldSize.z)
        );
        if (!Container.Contains(new Vector2(Brush.CurrentWorldPos.x, Brush.CurrentWorldPos.z)))
            return;

        RTHandle Temp = Brush.bIsPreview ? PreviewRTHandle : HeightRTHandle;
        Shader.SetTexture(PaintKernel, "Result", HeightRTHandle.rt);
        Shader.SetTexture(PaintKernel, "Preview", PreviewRTHandle.rt);
        Shader.SetVector("_TexSize", (Vector2)Settings.TexSize);
        Shader.SetVector("_WorldSize", (Vector3)Settings.WorldSize);
        Shader.SetVector("_WorldPos", transform.position);
        Shader.SetFloat("_BrushSize", Brush.Size);
        Shader.SetFloat("_BrushStrength", Brush.Strength);
        Shader.SetInt("_BrushType", Brush.Type);
        Shader.SetInt("_BrushOverrideType", Brush.OverrideType);
        Shader.SetBool("_BrushIsPreview", Brush.bIsPreview);
        Shader.SetBool("_BrushHasPreview", bHasPreview);
        Shader.SetVector("_MousePosition", Brush.CurrentWorldPos);
        Shader.SetVector("_MouseStartPosition", Brush.StartWorldPos);
        Shader.Dispatch(PaintKernel, Temp.rt.width, Temp.rt.height, 1);
        EditorUtility.SetDirty(TerrainMat);
        EditorUtility.SetDirty(this);

        bHasPreview = Brush.bIsPreview || bHasPreview;
        if (Brush.bIsPreview)
            return;

        GenerateMesh(false);
        ResetRT(true);
    }

    public void Reset()
    {
        ResetRT(true);
        ResetRT(false);
    }

    public void ResetRT(bool bIsPreview)
    {
        RTHandle Target = bIsPreview ? PreviewRTHandle : HeightRTHandle;
        Shader.SetTexture(ResetKernel, "Result", Target.rt);
        Shader.Dispatch(ResetKernel, Target.rt.width, Target.rt.height, 1);
        EditorUtility.SetDirty(this);
        bHasPreview = !bIsPreview && bHasPreview;
    }

    public void GenerateMesh(bool bAutoLoad = true)
    {
        if (HeightRTHandle == null)
        {
            Init();
        }
        if (Shader == null || HeightRTHandle == null)
            return;

        if (bAutoLoad && HasHeightTex())
        {
            LoadHeightTex();
        }

        TriangleAppendBuffer.SetCounterValue(0);
        Shader.SetVector("_WorldPos", transform.position);
        Shader.SetBuffer(GenerateHeightKernel, "TriangleBuffer", TriangleAppendBuffer);
        Shader.SetTexture(GenerateHeightKernel, "Result", HeightRTHandle.rt);

        Shader.Dispatch(GenerateHeightKernel, HeightRT.width - 1, HeightRT.height - 1, 1);

        // read out the actually found grass positions
        RenderArgsBuffer = new ComputeBuffer(1, Args.Length * sizeof(uint), ComputeBufferType.IndirectArguments);
        RenderArgsBuffer.SetData(Args);
        ComputeBuffer.CopyCount(TriangleAppendBuffer, RenderArgsBuffer, 0);
        RenderArgsBuffer.GetData(Args);
        var TriangleCount = Args[0];

        // now we can actually fill in the data
        Args[0] = TriangleCount * 3;
        Args[1] = 1;
        Args[2] = 0;
        Args[3] = 0;
        RenderArgsBuffer.SetData(Args);

        TerrainMat.SetTexture("_HeightTex", HeightRTHandle.rt);
        TerrainMat.SetTexture("_PreviewTex", PreviewRTHandle.rt);
        TerrainMat.SetBuffer("PositionBuffer", TriangleAppendBuffer);
        TerrainMat.SetFloat("_Width", Settings.TexSize.x);
        TerrainMat.SetVector("_WorldPos", transform.position);
    }

    public void Pixel()
    {
        Shader.SetVector("_TexSize", (Vector2)Settings.TexSize);
        Shader.SetVector("_WorldSize", (Vector3)Settings.WorldSize);
        Shader.SetTexture(PixelKernel, "Result", HeightRT);
        Shader.Dispatch(PixelKernel, HeightRT.width, HeightRT.height, 1);
    }


    public void OnDestroy()
    {
        Destroy();
    }

    public void SaveHeightTex()
    {
        var Tex = new Texture2D(HeightRT.width, HeightRT.height, textureFormat: TextureFormat.RGBA32, false);

        var asyncAction = AsyncGPUReadback.Request(HeightRT, 0);
        asyncAction.WaitForCompletion();
        Tex.SetPixelData(asyncAction.GetData<byte>(), 0);
        Tex.Apply();
        var Bytes = Tex.EncodeToPNG();

        var Path = GetHeightFilePath();
        if (File.Exists(Path))
        {
            File.Delete(Path);
        }
        File.WriteAllBytes(Path, Bytes);
    }

    private void LoadHeightTex()
    {
        if (HeightRTHandle == null || !HasHeightTex())
            return;

        var Bytes = File.ReadAllBytes(GetHeightFilePath());
        Texture2D Tex = new Texture2D(HeightRT.width, HeightRT.height, textureFormat: TextureFormat.RGBA32, false);
        Tex.LoadImage(Bytes, false);
        Tex.Apply();
        Graphics.Blit(Tex, HeightRT);
    }

    private String GetHeightFilePath() {
        return Application.dataPath + "/Resources" + HeightPath;
    }

    private bool HasHeightTex()
    {
        return File.Exists(GetHeightFilePath());
    }

    public Vector4 ProjectMouseOnPlane(SceneView View)
    {
        if (View == null)
            return Vector4.zero;

        Plane Plane = new(Vector3.up, 1);
        Vector4 Point = Vector4.zero;

        Ray Ray = HandleUtility.GUIPointToWorldRay(Event.current.mousePosition);
        if (Plane.Raycast(Ray, out float Enter))
        {
            Point = Ray.GetPoint(Enter);
            Point.w = 1;
        }
        return Point;
    }

    public void OnEnable()
    {
        GenerateDefaultMesh();
        GenerateMesh();
    }

    public bool IsInit()
    {
        return
            Shader != null &&
            HeightRTHandle != null &&
            RenderArgsBuffer != null &&
            TerrainMat != null &&
            TriangleAppendBuffer != null;
    }

    private void Destroy()
    {
        Graphics.SetRenderTarget(null);
        HeightRTHandle?.Release();
        PreviewRTHandle?.Release();
        PositionBuffer?.Release();
        RenderArgsBuffer?.Release();
        TriangleAppendBuffer?.Release();
        HeightRT = null;
        PreviewRT = null;
    }

    private int GetCount()
    {
        if (Settings == null)
            return 0;
        return VertexCount * Settings.TexSize.x * Settings.TexSize.y;
    }
}

[Serializable]
public struct TTBrushSettings
{
    public int Type;
    public int OverrideType;
    public float Size;
    public float Strength;
    public Vector4 CurrentWorldPos;
    public Vector4 StartWorldPos;
    public bool bIsPreview;
}