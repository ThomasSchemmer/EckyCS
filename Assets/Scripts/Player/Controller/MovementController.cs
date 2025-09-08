using System.Collections;
using System.Collections.Generic;
using Unity.Mathematics;
using UnityEditor;
using UnityEngine;

public class MovementController : IMovementComponent
{
    [Header("Movement")]
    public float MovementSpeed = 5;
    public float SprintMultiplier = 1.5f;
    public float CrouchMultiplier = 0.4f;
    public float JumpForce = 8;
    public float AirMultiplier = 0.4f;

    [Header("Crouch")]
    public float CrouchHeight = 0.4f;
    public float CrouchScaleSpeed = 3;

    [Header("Floating")]
    public float HoverDistance = 1f;
    public float SpringForce = 10;
    public float SpringDamp = 3;

    [Header("Camera")]
    public float Sensitity = 400;
    public float Smoothness = 1;

    private bool bIsJumpReady = true;
    private int JumpCounter = JumpInitialCounter;
    public Camera Cam;

    public override void BaseInit()
    {
        Controller.GetRigidbody().freezeRotation = true;
        Cam = Camera.main;
    }

    public override void UpdateState()
    {
#if UNITY_EDITOR
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            EditorApplication.ExitPlaymode();
        }
#endif

        bool bShouldResetJump = Controller.IsGrounded() && !Controller.IsJumping();
        JumpCounter = bShouldResetJump ? JumpInitialCounter : JumpCounter;
        Controller.AddDebugText("Jump: " + JumpCounter + "\n");
    }

    public override void FixedUpdateMovement()
    {
        Scale();
        Hover();
    }

    private void Hover()
    {
        if (!Controller.IsGrounded() || Controller.IsJumping() || Controller.IsHooking() || Controller.IsWallRunning())
            return;

        var Rb = Controller.GetRigidbody();
        Vector3 Down = transform.TransformDirection(Vector3.down);
        float Velocity = Vector3.Dot(Down, Rb.velocity);
        float Distance = Controller.GetGroundHit().distance - HoverDistance;
        float Force = (Distance * SpringForce) - (Velocity * SpringDamp);
        Rb.AddForce(Force * Down, ForceMode.Force);
    }

    private void Scale()
    {
        float Target = Controller.State == PlayerController.MovementState.Crouching ? CrouchHeight : 1;
        float y = Mathf.Lerp(transform.localScale.y, Target, Time.deltaTime * CrouchScaleSpeed);
        transform.localScale = new(transform.localScale.x, y, transform.localScale.z);
    }

    public override Vector3 GetMovement()
    {
        Vector3 Movement = new();
        float H = Input.GetAxis("Horizontal");
        float V = Input.GetAxis("Vertical");
        Movement += V * new Vector3(1, 0, 1);
        Movement += H * new Vector3(1, 0, -1);

        float MoveMulti = 1;
        switch (Controller.State)
        {
            case PlayerController.MovementState.Sprinting: MoveMulti = SprintMultiplier; break;
            case PlayerController.MovementState.Crouching: MoveMulti = CrouchMultiplier; break;
            case PlayerController.MovementState.Falling: MoveMulti = AirMultiplier; break;
        }

        float Multiplier = MovementSpeed * MoveMulti;
        Movement = Multiplier * Movement.normalized;

        return Movement;
    }

    public override bool ShouldProvideMovement()
    {
        return !Controller.IsHooking();
    }

    public override void OnCollisionEnter(Collision collision)
    {
        base.OnCollisionEnter(collision);
        JumpCounter = JumpInitialCounter;
    }

    public override void OnAfterJump()
    {
        base.OnAfterJump();

        bIsJumpReady = false;
        JumpCounter--;
        Invoke(nameof(ResetJump), 0.4f);
    }

    private void ResetJump()
    {
        bIsJumpReady = true;
    }

    public override string GetName()
    {
        return "Movem: ";
    }

    public override bool ShouldProvideJumpMovement()
    {
        return IsJumpReady() && !Controller.IsWallRunning();
    }

    public override Vector3 GetJumpMovement()
    {
        return Vector3.up * JumpForce;
    }

    public bool IsJumpReady()
    {
        return bIsJumpReady && JumpCounter > 0;
    }

    private static int JumpInitialCounter = 2;
}
