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

    public void Brush(SceneView View)
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
        Shader.SetVector("_MousePosition", Point);
        Shader.Dispatch(PaintKernel, HeightRT.width, HeightRT.height, 1);
        EditorUtility.SetDirty(TerrainMat);
        EditorUtility.SetDirty(this);
    }

    public void Reset()
    {
        Shader.SetTexture(ResetKernel, "Result", HeightRTHandle.rt);
        Shader.Dispatch(ResetKernel, HeightRT.width, HeightRT.height, 1);
    }

    GameObject Temp;
    public void Debug()
    {
        if (Temp)
        {
            DestroyImmediate(Temp);
        }
        Temp = GameObject.CreatePrimitive(PrimitiveType.Cube);
        Temp.GetComponent<MeshFilter>().mesh = CreateMesh();



        return;
        TriangleAppendBuffer = new ComputeBuffer(100000, sizeof(float) * 3 * 3, ComputeBufferType.Append);
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


    private Vector4 GetMousePoint(SceneView View)
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

    static int[][] Structures = new int[6][]
    {
        // flat bottom
        new int[6]{0, 1, 2, 1, 3, 2 },
        // corner raised
        new int[18]{0, 1, 2, 1, 7, 2, 1, 6, 7, 7, 6, 11, 6, 10, 11, 10, 15, 11 },
        // edge raised
        new int[18]{0, 1, 5, 1, 6, 5, 5, 6, 9, 6, 10, 9, 9, 10, 14, 14, 10, 15},
        // opposites raised
        new int[30]{0, 4, 5, 5, 4, 9, 8, 9, 4, 9, 11, 14, 9, 8, 11, 10, 11, 8, 8, 13, 10, 10, 6, 7, 7, 11, 10, 7, 6, 3},
        // corner lowered
        new int[18]{0, 4, 5, 5, 4, 9, 4, 8, 9, 8, 13, 14, 8, 14, 9, 13, 15, 14},
        // flat ceiling
        new int[6] { 12, 13, 14, 13, 15, 14},
    };


    /** contains structure index and y-rotation */
    static Vector2[] HeightToStructure = new Vector2[16]{
        new (0, 0),
        new (1, 0),
        new (1, 90),
        new (2, 0),
        new (1, -90),
        new (2, -90),
        new (3, 0),
        new (4, 0),
        new (1, 180),
        new (3, 90),
        new (2, 90),
        new (4, 90),
        new (2, 180),
        new (4, -90),
        new (4, 180),
        new (5, 0),
    };

    /** 
     * contains {Xpos, HeightMinMaxInd, ZPos, HeightIndexA, HeightIndexB}
     */
    static float[][] VertexToWorld = new float[16][]{
        new float[]{0, 0, 1, 0, 0},
        new float[]{1, 0, 1, 1, 1},
        new float[]{0, 0, 0, 2, 2},
        new float[]{1, 0, 0, 3, 3},
        new float[]{.5f, 0, 1, 0, 1},
        new float[]{0, 0, .5f, 0, 2},
        new float[]{1, 0, .5f, 1, 3},
        new float[]{.5f, 0, 0, 2, 3},
        new float[]{.5f, 1, 1, 0, 1},
        new float[]{0, 1, .5f, 0, 2},
        new float[]{1, 1, .5f, 1, 3},
        new float[]{.5f, 1, 0, 2, 3},
        new float[]{0, 1, 1, 0, 0},
        new float[]{1, 1, 1, 1, 1},
        new float[]{0, 1, 0, 2, 2},
        new float[]{1, 1, 0, 3, 3},
    };

    /** Uses height interpolation to calculate the resulting WorldPos*/
    Vector3 GetVertex(int Index, float[] Heights)
    {
        float[] Temp = VertexToWorld[Index];
        Vector3 Result = new(Temp[0], 0, Temp[2]);
        float HeightA = Heights[(int)Temp[3]];
        float HeightB = Heights[(int)Temp[4]];
        Result.y = Temp[1] * Mathf.Max(HeightA, HeightB) + (1 - Temp[1]) * Mathf.Min(HeightA, HeightB);
        return Result;
    }

    int Index(Vector2Int id, Vector2Int Offset)
    {
        return (id.y + Offset.y) * HeightRT.width + id.x + Offset.x;
    }

    Vector3 RotatePoint(Vector3 Point, Matrix4x4 Matrix, Vector2Int Offset)
    {
        Point -= new Vector3(.5f, 0, .5f);
        Point = Matrix.MultiplyPoint3x4(Point);
        Point += new Vector3(.5f, 0, .5f);
        return Point + new Vector3(Offset.x, 0, Offset.y);
    }

    float[] RotateHeights(float[] Heights, float YAngle)
    {
        float[] Result = new float[4];
        int[] Rotation = { 
            0, 1, 2, 3, 
            2, 0, 3, 1,
            3, 2, 1, 0,
            1, 3, 0, 2
        };
        int Offset = (int)(YAngle / 90f);
        Offset = (Offset + 4) % 4;
        for (int i = 0; i < Heights.Length; i++)
        {
            Result[i] = Heights[Rotation[i + Offset * 4]];
        }
        return Result;
    }

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
                Vector2Int id = new(x, y);
                // height indicates the type of mesh via lookup table
                // clamped to different height bands
                float Height0 = Pixels[Index(id, new Vector2Int(0, 1))].r;
                float Height1 = Pixels[Index(id, new Vector2Int(1, 1))].r;
                float Height2 = Pixels[Index(id, new Vector2Int(0, 0))].r;
                float Height3 = Pixels[Index(id, new Vector2Int(1, 0))].r;
                //Height0 = (int)(Height0 * HeightBand) / HeightBand;
                //Height1 = (int)(Height1 * HeightBand) / HeightBand;
                //Height2 = (int)(Height2 * HeightBand) / HeightBand;
                //Height3 = (int)(Height3 * HeightBand) / HeightBand;
                float HeightMin = Mathf.Min(Height0, Mathf.Min(Height1, Mathf.Min(Height2, Height3)));

                int HeightID =
                    ((Height0 > HeightMin) ? 1 : 0) << 3 |
                    ((Height1 > HeightMin) ? 1 : 0) << 2 |
                    ((Height2 > HeightMin) ? 1 : 0) << 1 |
                    ((Height3 > HeightMin) ? 1 : 0) << 0;

                if (HeightID > HeightToStructure.Length)
                    continue;

                Vector2 Info = HeightToStructure[HeightID];
                int[] VertexIndices = Structures[(int)Info.x];
                float YAngle = Info.y;

                // rotate heights to "original" version to allow base 
                // vertices to be calculated. Will be rotated back later
                float[] Heights = { Height0, Height1, Height2, Height3 };
                Heights = RotateHeights(Heights, -YAngle);

                var M = Matrix4x4.Rotate(Quaternion.Euler(0, YAngle, 0));

                // lookup the blueprint for the structure
                for (int i = 0; i < VertexIndices.Length; i += 3)
                {
                    if (VertexIndices[i] == -1)
                        break;

                    int TriCount = Vertices.Count;
                    Vector3 A = GetVertex(VertexIndices[i + 0], Heights);
                    Vector3 B = GetVertex(VertexIndices[i + 1], Heights);
                    Vector3 C = GetVertex(VertexIndices[i + 2], Heights);
                    A = RotatePoint(A, M, id);
                    B = RotatePoint(B, M, id);
                    C = RotatePoint(C, M, id);
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
