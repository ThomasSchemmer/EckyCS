using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BCurve : Curve
{

    protected override Vector4[] GetPolynomials()
    {
        return new Vector4[]{
            new(1, 4, 1, 0),
            new(-3, 0, 3, 0),
            new(3, -6, 3, 0),
            new(-1, 3, -3, 1)
        };
    }

    protected override float GetMultiplier()
    {
        return 1 / 6.0f;
    }

    protected override int GetDimension()
    {
        return BSpline.Dimension;
    }

}
