using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BezierSpline : Spline<BezierCurve>
{

    public override Color GetLineColor()
    {
        return Color.red;
    }
    public override int GetDimension()
    {
        return Dimension;
    }

    public static int Dimension = 3;
}
