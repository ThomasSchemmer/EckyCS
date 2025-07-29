using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.UIElements;

[ExecuteInEditMode]
public class TTManager : MonoBehaviour
{
    public TTSettings Settings;
    public float BrushSize = 0;

    public ComputeShader Shader;
    public Material TerrainMat;
    private ComputeBuffer PositionBuffer;
    private ComputeBuffer TriangleAppendBuffer;
    private uint[] Args = new uint[5] { 0, 1, 0, 0, 0 };
    private ComputeBuffer RenderArgsBuffer;
    private int GenerateKernel, PaintKernel, ResetKernel, DebugKernel, PixelKernel;
    private readonly int VertexCount = 6;

    public RenderTexture HeightRT;
    private RTHandle HeightRTHandle;

    public void OnRenderObject()
    {
        if (PositionBuffer == null || RenderArgsBuffer == null || TerrainMat == null)
            return;

        TerrainMat.SetPass(0);
        Graphics.DrawProceduralIndirectNow(MeshTopology.Triangles, RenderArgsBuffer);
    }

    public void Generate()
    {
        Init();
    }

    private void Init()
    {
        Destroy();
        if (Settings == null)
            return;

        HeightRTHandle = RTHandles.Alloc(width: Settings.Width, height: Settings.Height, enableRandomWrite: true, useMipMap: false);
        HeightRT = HeightRTHandle.rt;
        int Count = VertexCount * Settings.Width * Settings.Height;
        PositionBuffer = new(Count, sizeof(float) * 3); 

        RenderArgsBuffer = new ComputeBuffer(4, sizeof(int), ComputeBufferType.IndirectArguments);  
        RenderArgsBuffer.SetData(new int[] { Count, 1, 0, 0 });

        TriangleAppendBuffer = new ComputeBuffer(100000, sizeof(float) * 3 * 3, ComputeBufferType.Append);

        if (Shader == null)
            return;

        GenerateKernel = Shader.FindKernel("Generate");
        PaintKernel = Shader.FindKernel("Paint");
        ResetKernel = Shader.FindKernel("Reset");
        DebugKernel = Shader.FindKernel("Debug");
        PixelKernel = Shader.FindKernel("Pixel");

        Shader.SetFloat("_Width", Settings.Width);
        Shader.SetTexture(GenerateKernel, "Result", HeightRTHandle.rt);
        Shader.SetBuffer(GenerateKernel, "PositionBuffer", PositionBuffer);
        Shader.Dispatch(GenerateKernel, HeightRT.width - 1, HeightRT.height - 1, 1);

        TerrainMat.SetTexture("_HeightTex", HeightRTHandle.rt);
        TerrainMat.SetBuffer("PositionBuffer", PositionBuffer);
        TerrainMat.SetFloat("_Width", Settings.Width);
        Vector3[] Data = new Vector3[Count];
        PositionBuffer.GetData(Data);
    }

    public void OnSceneGUI(SceneView View)
    {
        TerrainMat.SetVector("_MousePosition", GetMousePoint(View));
        TerrainMat.SetFloat("_BrushSize", BrushSize);
    }

    public void Brush(SceneView View, float Strength)
    {
        Vector4 Point = GetMousePoint(View);
        if (Point.w == 0)
            return;

        Rect Container = new (Vector2.zero, new(Settings.Width, Settings.Height));
        if (!Container.Contains(new Vector2(Point.x, Point.z)))
            return;

        Shader.SetTexture(PaintKernel, "Result", HeightRTHandle.rt);
        Shader.SetFloat("_Width", Settings.Width);
        Shader.SetFloat("_BrushSize", BrushSize);
        Shader.SetFloat("_BrushStrength", Strength);
        Shader.SetVector("_MousePosition", Point);
        Shader.Dispatch(PaintKernel, HeightRT.width, HeightRT.height, 1);
        EditorUtility.SetDirty(TerrainMat);
        EditorUtility.SetDirty(this);
        Calculate();
    }

    public void Reset()
    {
        Shader.SetTexture(ResetKernel, "Result", HeightRTHandle.rt);
        Shader.Dispatch(ResetKernel, HeightRT.width, HeightRT.height, 1);
    }

    public void Calculate()
    {
        TriangleAppendBuffer.SetCounterValue(0);
        Shader.SetBuffer(DebugKernel, "TriangleBuffer", TriangleAppendBuffer);
        Shader.SetTexture(DebugKernel, "Result", HeightRTHandle.rt);

        Shader.Dispatch(DebugKernel, HeightRT.width - 1, HeightRT.height - 1, 1);

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
        TerrainMat.SetBuffer("PositionBuffer", TriangleAppendBuffer);
    }

    public void Pixel()
    {
        Shader.SetTexture(PixelKernel, "Result", HeightRT);
        Shader.Dispatch(PixelKernel, HeightRT.width, HeightRT.height, 1);
    }


    public void OnDestroy()
    {
        Destroy();
    }


    public Vector4 GetMousePoint(SceneView View)
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

    private void Destroy()
    {
        HeightRTHandle?.Release();
        PositionBuffer?.Release();
        RenderArgsBuffer?.Release();
        TriangleAppendBuffer?.Release();
        HeightRT = null;
    }

    /** Maps the height number into the corresponding vertex structure, see
     * https://www.boristhebrave.com/2021/12/29/2d-marching-cubes-with-multiple-colors/
     * -1 => invalid index
     */
    static int[] VertexMap = new int[]
    {
        //0, 1, -2, -3, 4, 5, 6, -7, -8, -9, -10, -11, -12, -13, -14, -15, 16, 17, 18, -19, 20, 21, 22, -23, 24, 25, 26, 27
          0, 1, -1, -1, 2, 3, 4, -1, -1, -1, -1,   -1,  -1,  -1,  -1,  -1,  5,  6,  7,  -1,  8,  9, 10,  -1, 11, 12, 13, 14
    };

    /** 
     * indices to vertices arranged in a plane
     * 0--4/8--1
     * |       |
     * 5/9-X---6/10
     * |       |
     * 2--7/11-3
     * where x is 12, 13, 14, 15
     * Index dictates the height lookup (ie 4-> spline 0 to 1, lower number so start height: v0)
     * Connecting triples into triangles results in the actual mesh part
     */
    static int[][] VertexLookup = new int[][]
    {
        new int[]{0, 1, 2, 1, 3, 2},
        new int[]{0, 1, 2, 1, 6, 7, 7, 2, 1, 6, 11, 7, 6, 10, 11, 10, 3, 11},
        new int[]{0, 1, 3, 0, 11, 5, 0, 3, 11, 7, 9, 11, 5, 11, 9, 9, 7, 2},
        new int[]{0, 1, 5, 1, 6, 5, 5, 10, 9, 6, 10, 5, 9, 10, 2, 2, 10, 3},
        new int[]{0, 1, 5, 1, 6, 5, 5, 12, 9, 12, 14, 9, 12, 6, 15, 6, 10, 15, 9, 14, 7, 7, 2, 9, 15, 10, 3, 3, 11, 15, 15, 14, 7, 7, 11, 15},
        new int[]{0, 3, 2, 0, 4, 10, 10, 3, 0, 4, 8, 6, 6, 10, 4, 8, 1, 6},
        new int[]{0, 4, 7, 7, 2, 0, 4, 8, 11, 11, 7, 4, 8, 1, 3, 3, 11, 8},
        new int[]{0, 4, 7, 7, 2, 0, 4, 8, 13, 13, 12, 4, 14, 15, 11, 11, 7, 14, 15, 10, 3, 3, 11, 15, 8, 1, 6, 6, 13, 8, 13, 6, 10, 10, 15, 13},
        new int[]{0, 4, 5, 4, 8, 9, 9, 5, 4, 8, 1, 6, 7, 2, 9, 9, 8, 6, 6, 7, 9, 7, 6, 10, 10, 11, 7, 10, 3, 11},
        new int[]{0, 4, 5, 4, 8, 9, 9, 5, 4, 8, 1, 6, 8, 6, 7, 7, 9, 8, 9, 7, 2, 10, 3, 11},
        new int[]{0, 4, 5, 4, 8, 9, 9, 5, 4, 8, 1, 6, 7, 2, 9, 9, 8, 6, 6, 7, 9, 7, 6, 10, 10, 11, 7, 10, 3, 11},
        new int[]{0, 4, 5, 4, 10, 11, 11, 5, 4, 10, 3, 11, 4, 8, 6, 6, 10, 4, 5, 11, 7, 7, 9, 5, 9, 7, 2, 8, 1, 6},
        new int[]{8, 1, 3, 3, 11, 8, 4, 8, 13, 13, 12, 4, 14, 15, 11, 11, 7, 14, 0, 4, 12, 12, 5, 0, 9, 14, 7, 7, 2, 9, 5, 12, 14, 14, 9, 5},
        new int[]{9, 10, 3, 3, 2, 9, 5, 9, 14, 14, 12, 5, 15, 10, 6, 6, 13, 15, 0, 4, 12, 12, 5, 0, 8, 1, 6, 6, 13, 8, 4, 8, 13, 13, 12, 4},
        new int[]{0, 4, 12, 12, 5, 0, 8, 1, 6, 6, 13, 8, 15, 10, 3, 3, 11, 15, 7, 2, 9, 9, 14, 7, 9, 5, 12, 12, 14, 9, 4, 8, 13, 13, 12, 4, 13, 6, 10, 10, 15, 13, 11, 7, 14, 14, 15, 11}
    };

    /** 
     * contains {Xpos, ZPos, HeightLookup}
     */
    static float[][] VertexToWorld = new float[16][]{
        new float[]{0, 1, 0},
        new float[]{1, 1, 1},
        new float[]{0, 0, 2},
        new float[]{1, 0, 3},

        new float[]{.5f, 1, 0},
        new float[]{0, .5f, 0},
        new float[]{1, .5f, 1},
        new float[]{.5f, 0, 2},

        new float[]{.5f, 1, 1},
        new float[]{0, .5f, 2},
        new float[]{1, .5f, 3},
        new float[]{.5f, 0, 3},

        new float[]{.5f, .5f, 0},
        new float[]{.5f, .5f, 1},
        new float[]{.5f, .5f, 2},
        new float[]{.5f, .5f, 3},
    };

    /** Uses height interpolation to calculate the resulting WorldPos*/
    Vector3 GetVertex(int Index, int[] Heights)
    {
        float[] Temp = VertexToWorld[Index];
        Vector3 Result = new(Temp[0], Heights[(int)Temp[2]], Temp[1]);
        return Result;
    }

    int Index(Vector2Int id, Vector2Int Offset)
    {
        return (id.y + Offset.y) * HeightRT.width + id.x + Offset.x;
    }

    int Bands = 4;

    private Mesh CreateMesh()
    {
        var Tex = new Texture2D(HeightRT.width, HeightRT.height, textureFormat: TextureFormat.RGBA32, false);

        var asyncAction = AsyncGPUReadback.Request(HeightRT, 0);
        asyncAction.WaitForCompletion();
        Tex.SetPixelData(asyncAction.GetData<byte>(), 0);
        Tex.Apply();
        Color[] Pixels = Tex.GetPixels();
        List<Vector3> Vertices = new();
        List<int> Triangles = new();

        for (int x = 0; x < HeightRT.width - 1; x++)
        {
            for (int y = 0; y < HeightRT.height - 1; y++)
            {
                Vector2Int ID = new(x, y);
                Vector3 IDOffset = new(x, 0, y);
                // height indicates the type of mesh via lookup table
                // clamped to different height bands
                int Height0 = Mathf.RoundToInt(Pixels[Index(ID, new Vector2Int(0, 1))].r * (Bands - 1));
                int Height1 = Mathf.RoundToInt(Pixels[Index(ID, new Vector2Int(1, 1))].r * (Bands - 1));
                int Height2 = Mathf.RoundToInt(Pixels[Index(ID, new Vector2Int(0, 0))].r * (Bands - 1));
                int Height3 = Mathf.RoundToInt(Pixels[Index(ID, new Vector2Int(1, 0))].r * (Bands - 1));
                int[] HeightMap = new int[Bands];
                int[] Heights = { Height0, Height1, Height2, Height3 };
                for (int i = 0; i < Bands; i++)
                {
                    HeightMap[i] = -1;
                }
                int CurIndex = 0;
                HeightMap[Height0] = HeightMap[Height0] < 0 ? CurIndex++ : HeightMap[Height0];
                HeightMap[Height1] = HeightMap[Height1] < 0 ? CurIndex++ : HeightMap[Height1];
                HeightMap[Height2] = HeightMap[Height2] < 0 ? CurIndex++ : HeightMap[Height2];
                HeightMap[Height3] = HeightMap[Height3] < 0 ? CurIndex++ : HeightMap[Height3];

                int HeightID =
                    HeightMap[Height3] + HeightMap[Height2] * 4 + HeightMap[Height1] * 16;

                if (HeightID > VertexMap.Length)
                    continue;

                int[] VertexIndices = VertexLookup[VertexMap[HeightID]];

                // lookup the blueprint for the structure
                for (int i = 0; i < VertexIndices.Length; i += 3)
                {
                    if (VertexIndices[i] == -1)
                        break;

                    int TriCount = Vertices.Count;
                    Vector3 A = GetVertex(VertexIndices[i + 0], Heights) + IDOffset;
                    Vector3 B = GetVertex(VertexIndices[i + 1], Heights) + IDOffset;
                    Vector3 C = GetVertex(VertexIndices[i + 2], Heights) + IDOffset;

                    Vertices.Add(A);
                    Vertices.Add(B);
                    Vertices.Add(C);
                    Triangles.Add(TriCount + 0);
                    Triangles.Add(TriCount + 1);
                    Triangles.Add(TriCount + 2);
                }
            }
        }

        Mesh Mesh = new();
        Mesh.vertices = Vertices.ToArray();
        Mesh.triangles = Triangles.ToArray();
        Mesh.RecalculateNormals();
        return Mesh;
    }
}
