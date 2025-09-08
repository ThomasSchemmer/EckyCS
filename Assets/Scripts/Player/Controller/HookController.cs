using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HookController : IMovementComponent
{
    public LineRenderer LineRenderer;
    public GameObject HookTarget;

    private Camera Cam;
    private RaycastHit Target;
    private float OvershootYAxis = 5;
    private bool bIsHooking = false;
    private bool bResetOnImpact = false;

    public override void BaseInit()
    {
        Cam = Camera.main;
    }

    public void StartHook(RaycastHit Hit)
    {
        Target = Hit;
        bIsHooking = true;
        bResetOnImpact = true;
        LineRenderer.enabled = true;
        ApplyJump();
    }

    public override void UpdateState()
    {
        Shoot();
        UpdateLine();
    }

    public override void FixedUpdateMovement()
    {

    }

    private void Shoot()
    {
        return;
        if (bIsHooking)
            return;

        bool bHasHit = Physics.Raycast(Cam.transform.position, Cam.transform.forward, out var Hit, Controller.GroundMask);
        DisplayTarget(Hit, bHasHit);
        if (!bHasHit)
            return;

        if (!Input.GetMouseButtonDown(0))
            return;

        StartHook(Hit);
    }

    private void DisplayTarget(RaycastHit Hit, bool bHasHit)
    {
        HookTarget.SetActive(bHasHit);
        if (!bHasHit)
            return;

        HookTarget.transform.position = Hit.point + Hit.normal;
        HookTarget.transform.rotation = Quaternion.LookRotation(transform.forward, Vector3.up);
    }

    private void UpdateLine()
    {
        if (!bIsHooking)
            return;

        LineRenderer.SetPosition(0, LineRenderer.transform.position);
        LineRenderer.SetPosition(1, Target.point);
    }

    private void ApplyJump()
    {
        // see sebastian lague video 
        Vector3 lowestPoint = new Vector3(transform.position.x, transform.position.y - 1f, transform.position.z);

        float grapplePointRelativeYPos = Target.point.y - lowestPoint.y;
        float highestPointOnArc = grapplePointRelativeYPos + OvershootYAxis;

        if (grapplePointRelativeYPos < 0) highestPointOnArc = OvershootYAxis;

        Vector3 Velocity = CalculateJumpVelocity(transform.position, Target.point, highestPointOnArc);
        return;
        int asd = 5;
        //Controller.SetVelocity(Velocity);
    }


    public Vector3 CalculateJumpVelocity(Vector3 StartPoint, Vector3 EndPoint, float Height)
    {
        float Gravity = Physics.gravity.y;
        Vector3 Displacement = EndPoint - StartPoint;

        float Time = Mathf.Sqrt(-2 * Height / Gravity) + Mathf.Sqrt(2 * (Displacement.y - Height) / Gravity);
        Vector3 VelocityY = Vector3.up * Mathf.Sqrt(-2 * Gravity * Height);
        Vector3 VelocityXZ = new Vector3(Displacement.x, 0, Displacement.z) / Time;

        return VelocityXZ + VelocityY;
    }

    public bool IsHooking()
    {
        return bIsHooking;
    }

    public override void OnCollisionEnter(Collision Collision)
    {
        base.OnCollisionEnter(Collision);
        if (!bResetOnImpact || !bIsHooking)
            return;

        bIsHooking = false;
        LineRenderer.enabled = false;
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
        return "Hook: ";
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
