using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BezierCurve : Curve
{
    protected override int GetDimension()
    {
        return BezierSpline.Dimension;
    }

    protected override float GetMultiplier()
    {
        return 1;
    }

    protected override Vector4[] GetPolynomials()
    {
        return new Vector4[]{
            new (1, 0, 0, 0),
            new (-2, 2, 0, 0),
            new (1, -2, 1, 0),
        };
    }
}
