using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public abstract class Spline : MonoBehaviour
{
    public bool bIsEditable = false;
    [HideInInspector]
    public float t = 0;
    public List<Point> Points = new();

    public abstract Vector3 GetAt(float t);

    public abstract Vector3 DeriveAt(float t);

    public abstract Vector3 GetTangentAt(float t);

    public abstract Vector3 GetNormalAt(float t);

    public abstract float GetCurvatureAt(float t);
    public abstract float GetTimeOffsetAt(float SumT, float T);
    public abstract float GetMaxIndex();

    public abstract void Generate();

    public abstract Vector3[] GetSplineData();

    public void Update()
    {
        if (!Input.GetMouseButtonDown(0) || !bIsEditable)
            return;

        var Ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        if (!Physics.Raycast(Ray, out var Hit))
            return;

        Points.Add(new(Hit.point - Ray.direction));
    }



    public static int Accuracy = 100;
    public delegate void OnGenerated();
    public OnGenerated _OnGenerated;

}

public abstract class Spline<T> : Spline where T : Curve
{
    public List<T> Curves = new();

    public override Vector3 GetAt(float t)
    {
        if (!TryGetIndices(t, out var ti, out var tf))
            return Vector3.zero;

        return Curves[ti].GetAt(tf);
    }

    public override Vector3 DeriveAt(float t)
    {
        if (!TryGetIndices(t, out var ti, out var tf))
            return Vector3.zero;

        return Curves[ti].DeriveAt(tf);
    }

    public override Vector3 GetTangentAt(float t)
    {
        if (!TryGetIndices(t, out var ti, out var tf))
            return Vector3.zero;

        return Curves[ti].GetTangentAt(tf);
    }

    public override Vector3 GetNormalAt(float t)
    {
        if (!TryGetIndices(t, out var ti, out var tf))
            return Vector3.zero;

        return Curves[ti].GetNormalAt(tf);
    }

    public override float GetCurvatureAt(float t)
    {
        if (!TryGetIndices(t, out var ti, out var tf))
            return 0;

        return Curves[ti].GetCurvatureAt(tf);
    }

    private bool TryGetIndices(float t, out int ti, out float tf)
    {
        ti = (int)t;
        tf = t - ti;
        return !float.IsNaN(t) && Curves.Count > 0 && ti < Curves.Count;
    }

    public override float GetTimeOffsetAt(float SumT, float T)
    {
        if (!TryGetIndices(SumT, out var ti, out var _))
            return 0;

        return BCurve.StandardDistance / Curves[ti].GetLength() * T;
    }

    public void OnDrawGizmos()
    {
        if (Points == null || Points.Count < (GetDimension() - 1) || Curves == null || Curves.Count < 1)
            return;

        Gizmos.color = Color.white;
        for (int i = 0; i < Points.Count; i++)
        {
            Gizmos.DrawWireSphere(Points[i].Position, 0.5f);
        }

        Gizmos.color = GetLineColor();
        for (int i = 0; i < Accuracy * GetMaxIndex() - 1; i++)
        {
            float t0 = (i + 0) / (float)Accuracy;
            float t1 = (i + 1) / (float)Accuracy;
            Vector3 A = GetAt(t0);
            Vector3 B = GetAt(t1);
            Gizmos.DrawLine(A, B);
        }

        Vector3 Pos = GetAt(t);
        Vector3 Norm = GetNormalAt(t);
        Gizmos.color = Color.blue;
        Gizmos.DrawLine(Pos, Pos + Norm);
    }

    public override float GetMaxIndex()
    {
        if (Curves == null)
            return 0;

        return Curves.Count;
    }

    public override void Generate()
    {
        if (Points.Count < GetDimension())
            return;

        int CurveCount = Points.Count - (GetDimension() - 1);
        Curves = new();
        for (int i = 0; i < CurveCount; i++)
        {
            var Curve = (T)Activator.CreateInstance(typeof(T));
            Curve.Init(this, i);
            Curves.Add(Curve);
        }
        _OnGenerated?.Invoke();
    }

    public override Vector3[] GetSplineData()
    {
        Vector3[] Points = new Vector3[(int)GetMaxIndex() * 4];
        for (int i = 0; i < Points.Count(); i++)
        {
            Points[i] = Curves[(int)(i / 4.0f)].GetBasePointAt(i % 4);
        }
        return Points;
    }


    public abstract Color GetLineColor();
    public abstract int GetDimension();
}
