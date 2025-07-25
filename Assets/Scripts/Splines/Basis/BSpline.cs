using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class BSpline : Spline<BCurve>
{
    public Mesh Mesh;
    public Material Mat;

    public void Convert()
    {
        if (transform.childCount > 0)
        {
            DestroyImmediate(transform.GetChild(0).gameObject);
        }

        Generate();
        GameObject Child = new("Bezier");
        Child.transform.SetParent(transform, false);
        BezierSpline OtherSpline = Child.AddComponent<BezierSpline>();
        OtherSpline.Points = new List<Point>();
        OtherSpline.Points.Add(new(GetAt(0)));
        OtherSpline.Points.Add(new(GetAt(0.5f)));
        OtherSpline.Points.Add(new(GetAt((Accuracy - 1) / 100.0f)));
        OtherSpline.Generate();
    }

    public override Color GetLineColor()
    {
        return Color.white;
    }

    public override int GetDimension()
    {
        return Dimension;
    }


    public static int Dimension = 4;
}
