using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[RequireComponent(typeof(BSpline))]
[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
public class BSplineVisualizer : MonoBehaviour
{
    public Material Mat;
    private BSpline Spline;
    private MeshFilter MeshFilter;
    private MeshRenderer MeshRenderer;
    private Mesh Mesh;

    void Start()
    {
        Spline = GetComponent<BSpline>();
        MeshFilter = GetComponent<MeshFilter>();
        MeshRenderer = GetComponent<MeshRenderer>();
        Mesh = new();
        MeshFilter.mesh = Mesh;
        MeshRenderer.material = Mat;
        Spline._OnGenerated += Init;
    }

    private void Init()
    {
    }

    private void Update()
    {
        if (Spline == null)
            return;

        foreach (var Curve in Spline.Curves)
        {
            Curve.GetMesh(out var Vertices, out var Triangles);
            Mesh.vertices = Vertices.ToArray();
            Mesh.triangles = Triangles.ToArray();
            Mesh.RecalculateNormals();
        }
    }

    public void OnDrawGizmos()
    {
        if (Spline == null ||Spline.Curves.Count == 0)
            return;

        Gizmos.color = Color.magenta;
        //Curve.GetBoundingBox(out Vector3 Pos, out Vector3 Bounds);
        //Gizmos.DrawCube(transform.position + Pos, Bounds);
        Spline.Curves[0].GetMesh(out var Vertices, out var _);
        foreach (var Vertex in Vertices)
        {
            Gizmos.DrawCube(Vertex, Vector3.one * 0.1f);
        }
    }


    private void OnDestroy()
    {
        if (Spline == null)
            return;

        Spline._OnGenerated -= Init;
    }
}
