using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WallrunningController : IMovementComponent
{
    public float WallJumpForce = 3;

    public override void BaseInit()
    {

    }

    public override void UpdateState()
    {
    }


    public override void FixedUpdateMovement()
    {

    }

    public override bool ShouldProvideMovement()
    {
        return Controller.IsWallRunning();
    }

    public override Vector3 GetMovement()
    {
        var Rb = Controller.GetRigidbody();
        Rb.velocity = new Vector3(Rb.velocity.x, 0, Rb.velocity.z);
        Vector3 Movement = Controller.GetMovementComponent().GetMovement();
        Movement.y = 0;
        Movement = Vector3.ProjectOnPlane(Movement, Controller.GetWallHit().normal);
        return Movement;
    }

    public override bool ShouldProvideJumpMovement()
    {
        return Controller.IsWallRunning() && Controller.GetMovementComponent().IsJumpReady();
    }

    public override Vector3 GetJumpMovement()
    {
        Vector3 SelfOnWallDir = Vector3.ProjectOnPlane(transform.forward, Controller.GetWallHit().normal);
        Vector3 WallDir = Controller.GetWallHit().normal;
        Vector3 Dir = (WallDir + SelfOnWallDir + Vector3.up).normalized;
        return Dir * WallJumpForce;
    }

    public override string GetName()
    {
        return "Wallr: ";
    }
}
