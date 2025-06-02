using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public abstract class Curve
{
    protected float Length;
    protected Vector3[] Points;
    protected Vector3[] BasePoints;
    protected Vector3 BaseOffset = new();


    public Vector3 GetAt(float t)
    {
        t = Mathf.Clamp01(t);
        Vector3 Result = Vector3.zero;
        for (int i = 0; i < GetDimension(); i++)
        {
            float Temp = Mathf.Pow(t, i);
            Result += Points[i] * Temp;
        }
        return GetMultiplier() * Result + BaseOffset;
    }

    public Vector3 DeriveAt(float t)
    {
        t = Mathf.Clamp01(t);
        Vector3 Result = Vector3.zero;
        for (int i = 1; i < GetDimension(); i++)
        {
            float Temp = Mathf.Pow(t, i - 1) * i;
            Result += Points[i] * Temp;
        }
        return GetMultiplier() * Result;
    }

    public Vector3 DeriveTwiceAt(float t)
    {
        t = Mathf.Clamp01(t);
        Vector3 Result = Vector3.zero;
        for (int i = 2; i < GetDimension(); i++)
        {
            float Temp = Mathf.Pow(t, i - 2) * i * (i - 1);
            Result += Points[i] * Temp;
        }
        return GetMultiplier() * Result;
    }

    public float GetCurvatureAt(float t)
    {
        Vector3 Vel = DeriveAt(t);
        Vector3 Acc = DeriveTwiceAt(t);
        float A = Vector3.Magnitude(Vector3.Cross(Vel, Acc));
        float B = Mathf.Pow(Vector3.Magnitude(Vel), 3);
        return A / B;
    }

    public Vector3 GetTangentAt(float t)
    {
        return Vector3.Normalize(DeriveAt(t));
    }

    public Vector3 GetNormalAt(float t)
    {
        Vector3 A = GetAt(t);
        Vector3 B = GetAt(t + NormalOffset);
        Vector3 Ax = GetTangentAt(t);
        Vector3 Bx = Vector3.Normalize(GetTangentAt(t + NormalOffset) - (A - B));
        Vector3 Axis = Vector3.Normalize(Vector3.Cross(Bx, Ax));
        return Vector3.Cross(Axis, Ax);
    }

    public Vector3 GetBasePointAt(int i)
    {
        return BasePoints[i];
    }

    public float GetLength() {  return Length; }

    protected abstract Vector4[] GetPolynomials();
    protected abstract float GetMultiplier();
    protected abstract int GetDimension();

    public void Init(Spline Spline, int Start)
    {

        Points = new Vector3[GetDimension()];
        for (int i = 0; i < Points.Length; i++)
        {
            Points[i] = GetPoint(Start, i, Spline);
        }

        BaseOffset = Spline.transform.position;

        BasePoints = new Vector3[GetDimension()];
        for (int i = 0; i < BasePoints.Length; i++)
        {
            BasePoints[i] = Spline.Points[Start + i].Position;
        }

        Length = 0;
        for (int i = 0; i < Spline.Accuracy - 1; i++)
        {
            float t0 = (i + 0) / (float)Spline.Accuracy;
            float t1 = (i + 1) / (float)Spline.Accuracy;
            Vector3 Pos0 = GetAt(t0);
            Vector3 Pos1 = GetAt(t1);
            Length += Vector3.Distance(Pos0, Pos1);
        }
    }

    private Vector3 GetPoint(int Start, int Index, Spline Spline)
    {
        Vector4[] Polys = GetPolynomials();

        Vector3 Result =
            Polys[Index].x * Spline.Points[Start + 0].Position +
            Polys[Index].y * Spline.Points[Start + 1].Position +
            Polys[Index].z * Spline.Points[Start + 2].Position;

        if (GetDimension() == 3)
            return Result;

        return Result + Polys[Index].w * Spline.Points[Start + 3].Position;

    }

    public void GetBoundingBox(out Vector3 Position, out Vector3 Bounds)
    {
        Vector3 MinBounds = Vector3.zero;
        Vector3 MaxBounds = Vector3.zero;
        Position = Vector3.zero;
        foreach (var Point in BasePoints)
        {
            MinBounds = Vector3.Min(MinBounds, Point);
            MaxBounds = Vector3.Max(MaxBounds, Point);
        }
        Bounds = MaxBounds - MinBounds;
        Position = (MaxBounds - MinBounds) / 2f + MinBounds;
    }

    public void GetMesh(out List<Vector3> Vertices, out List<int> Triangles)
    {
        Vertices = new List<Vector3>();
        Triangles = new List<int>();

        for (int i = 0; i < BasePoints.Length; i++) 
        {
            bool bIsLast = i == BasePoints.Length - 1;
            int CurrentIndex = i;
            int NextIndex = bIsLast ? i  - 1: i + 1;
            Vector3 Current = BasePoints[CurrentIndex];
            Vector3 Next = BasePoints[NextIndex];
            Vector3 Dir = Next - Current;
            Vector3 Right = Vector3.Cross(Dir.normalized, -Camera.main.transform.forward).normalized;
            Right *= bIsLast ? -1 : 1;
            Vertices.Add(Current + Right);
            Vertices.Add(Current - Right);
        }

        for (int i = 0; i < BasePoints.Length - 1; i++)
        {
            Triangles.AddRange(new int[] { 2 * i + 0, 2 * i + 2, 2 * i + 1 });
            Triangles.AddRange(new int[] { 2 * i + 2, 2 * i + 3, 2 * i + 1 });
        }
    }

    public static float NormalOffset = 0.0001f;
    public static float StandardDistance = 5;
}
