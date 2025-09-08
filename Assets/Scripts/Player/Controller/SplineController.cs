using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

public class SplineController : IMovementComponent
{
    public TMPro.TextMeshProUGUI DebugText;

    [Range(0, 0.001f)]
    public float SplineDrag = 0.001f;
    [Range(0, 20)]
    public float SpeedMultiplier = 2;
    public BSpline Spline;

    private float t = 0;
    private float Velocity = 1;

    public override void BaseInit()
    {
        if (Spline != null)
        {
            Spline.Generate();
        }
    }

    public override void UpdateState()
    {
        UpdateSplineMode();
    }

    private void UpdateSplineMode()
    {
        if (Spline == null)
            return;

        float Increase = Vector3.Dot(Spline.GetTangentAt(t), -Vector3.up);
        Velocity += Increase * Time.deltaTime * SpeedMultiplier;
        Velocity = Mathf.Clamp(Velocity * (1 - SplineDrag), 0.75f, 3);
        DebugText.text = "Vel: " + Velocity + "\nInc: " + Increase;

        float Offset = Spline.GetTimeOffsetAt(t, Time.deltaTime) * Velocity;
        t += Offset;
        t %= Spline.GetMaxIndex();

        Vector3 Pos = Spline.GetAt(t);
        Vector3 Dir = Spline.GetTangentAt(t);
        Vector3 Norm = Spline.GetNormalAt(t);
        float Curvature = Spline.GetCurvatureAt(t);
        Vector3 Target = Vector3.Lerp(Vector3.up, -Norm, Curvature);
        Target = Vector3.Lerp(transform.up, Target, Time.deltaTime);

        transform.position = Pos;
        transform.rotation = Quaternion.LookRotation(Dir, Target);
    }

    public override void FixedUpdateMovement()
    {

    }

    public override bool ShouldProvideMovement()
    {
        return false;
    }

    public override Vector3 GetMovement()
    {
        return Vector3.zero;
    }
    public override string GetName()
    {
        return "Spline: ";
    }
    public override bool ShouldProvideJumpMovement()
    {
        return false;
    }

    public override Vector3 GetJumpMovement()
    {
        return Vector3.zero;
    }
}
