using System.Collections;
using System.Collections.Generic;
using System.Data;
using TMPro;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;

public class PlayerController : MonoBehaviour
{
    public enum MovementState
    {
        Walking,
        Sprinting,
        Crouching,
        Falling,
        Hooking,
        WallRunning
    }

    [Header("State")]
    public MovementState State = MovementState.Walking;
    public LayerMask GroundMask;

    [Header("KeyConfig")]
    public KeyCode JumpKey = KeyCode.Space;
    public KeyCode SprintKey = KeyCode.LeftShift;
    public KeyCode CrouchKey = KeyCode.LeftControl;

    [Header("Debug")]
    public TMPro.TextMeshProUGUI DebugText;
    public TMPro.TextMeshProUGUI FixedDebugText;

    private MovementController MovementComp;
    private HookController Hook;
    private SplineController Spline;
    private WallrunningController WallRunning;
    private List<IMovementComponent> IMovements = new();
    private Rigidbody Rb;

    private Vector3 Movement = new();
    private Vector3 GroundNormal = new();
    private Vector3 WallNormal = new();
    private RaycastHit GroundHit, WallLeftHit, WallRightHit;
    private List<Vector3> MoveList;

    private bool bIsSprinting = false;
    private bool bIsJumping = false;
    private bool bIsOnSlope = false;
    private bool bIsGrounded = false;
    private bool bIsHooking = false;
    private bool bIsCrouching = false;
    private bool bIsWallrunning = false;
    private bool bIsWallLeft, bIsWallRight;

    void Start()
    {
        Rb = GetComponent<Rigidbody>();
        MovementComp = GetComponent<MovementController>();
        Hook = GetComponent<HookController>();
        Spline = GetComponent<SplineController>();
        WallRunning = GetComponent<WallrunningController>();
        if (MovementComp != null)
            IMovements.Add(MovementComp);
        if (Hook != null)
            IMovements.Add(Hook);
        if (Spline != null)
            IMovements.Add(Spline);
        if (WallRunning != null)
            IMovements.Add(WallRunning);

        IMovements.ForEach(_ => _.Init());

        DebugText = GameObject.Find("UI/UpdateDebug").GetComponent<TextMeshProUGUI>();
        FixedDebugText = GameObject.Find("UI/FixedDebug").GetComponent<TextMeshProUGUI>();
    }

    void Update()
    {
        UpdateState();

        foreach(var IMovement in IMovements)
        {
            if (!IMovement.ShouldProvideMovement())
                continue;

            Vector3 Temp = IMovement.GetMovement();
            if (Temp.magnitude == 0)
                return;

            Movement += Temp;
        }
    }

    private void FixedUpdate()
    {
        FixedDebugText.text = "";
        Move();
        Rotate();
        Jump();
        UpdateFixedMovement();
        IMovements.ForEach(_ => _.FixedUpdateMovement());
    }

    private void Move()
    {

        Vector3 NormMove = Vector3.ProjectOnPlane(Movement, GetGroundNormal());
        Movement = new();

        MoveList = CollideAndSlide(NormMove);
        foreach (var Move in MoveList)
        {
            Rb.transform.position += Move;
        }
    }

    private void Rotate()
    {
        var Comp = GetMovementComponent();
        Plane Plane = new(Vector3.up, 1);
        var Ray = Comp.Cam.ScreenPointToRay(Input.mousePosition);
        if (!Plane.Raycast(Ray, out float Distance))
            return;

        Vector3 TargetPoint = Ray.GetPoint(Distance);
        TargetPoint.y = transform.position.y;
        Vector3 TargetDir = TargetPoint - transform.position;
        Quaternion TargetRotation = Quaternion.LookRotation(TargetDir, Vector3.up);
        transform.rotation = Quaternion.Lerp(transform.rotation, TargetRotation, Time.deltaTime * Comp.Smoothness);
    }

    private void Jump()
    {

        if (!IsJumping())
            return;

        foreach (var IMovement in IMovements)
        {
            if (!IMovement.ShouldProvideJumpMovement())
                continue;

            Rb.velocity = new Vector3(Rb.velocity.x, 0, Rb.velocity.z);
            Rb.AddForce(IMovement.GetJumpMovement(), ForceMode.Impulse);
            IMovement.OnAfterJump();
        }
    }

    private void UpdateFixedMovement()
    {
        return;
        Rb.useGravity = !bIsWallrunning || State == MovementState.Falling;
 
    }

    private void UpdateState()
    {
        DebugText.text = "";
        ShootRays();
        GroundNormal = bIsGrounded ? GroundHit.normal : Vector3.up;
        float Angle = Vector3.Angle(Vector3.up, GroundNormal);
        bIsOnSlope = Angle > 1 && Angle < 40;
        bIsHooking = GetHook().IsHooking();
        bIsCrouching = Input.GetKey(CrouchKey);
        bIsJumping = Input.GetKey(JumpKey);
        bIsSprinting = Input.GetKeyDown(SprintKey) ? !bIsSprinting : bIsSprinting;

        if (bIsHooking)
        {
            State = MovementState.Hooking;
        }
        else if (!bIsGrounded && bIsWallrunning)
        {
            State = MovementState.WallRunning;
        }
        else if (!bIsGrounded)
        {
            State = MovementState.Falling;
        }
        else if (bIsCrouching)
        {
            State = MovementState.Crouching;
        }
        else if (bIsSprinting)
        {
            State = MovementState.Sprinting;
        }
        else
        {
            State = MovementState.Walking;
        }

        foreach (var IMovement in IMovements)
        {
            IMovement.UpdateState();
        }
    }

    private void ShootRays()
    {
        bIsGrounded = Physics.Raycast(
            GetGroundCastStart(),
            Vector3.down,
            out GroundHit,
            1
        );

        if (bIsGrounded)
            return;

        bIsWallLeft = Physics.Raycast(
            transform.position,
            -transform.right,
            out WallLeftHit,
            1,
            GroundMask);
        bIsWallRight = Physics.Raycast(
            transform.position,
            transform.right,
            out WallRightHit,
            1,
            GroundMask);
        bIsWallrunning = bIsWallLeft || bIsWallRight;
    }


    public HookController GetHook()
    {
        return Hook;
    }

    public MovementController GetMovementComponent()
    {
        return MovementComp;
    }
    public Vector3 GetGroundCastStart()
    {
        return transform.position + Vector3.down * (transform.localScale.y - 0.1f);
    }

    public bool IsIn(MovementState Target)
    {
        return State == Target;
    }

    private void OnCollisionEnter(Collision Collision)
    {
        Hook.OnCollisionEnter(Collision);
    }

    private List<Vector3> CollideAndSlide(Vector3 Movement)
    {
        List<Vector3> SplitMoves = new();
        if (Vector3.Magnitude(Movement) == 0)
            return SplitMoves;

        float Radius = Rb.transform.localScale.z / 2f;
        Vector3 PredPos = Rb.transform.position;
        Movement = PredictDistance(Movement);
        float SkinDepth = 0.01f;
        float Range = Movement.magnitude + Radius + SkinDepth;
        WallNormal = new();

        int MaxCollision = 5;
        for (int i = 0; i < MaxCollision; i++)
        {
            if (!Physics.SphereCast(PredPos, Radius - SkinDepth, Movement.normalized, out var Hit, Range, GroundMask))
            {
                SplitMoves.Add(Movement);
                return SplitMoves;
            }

            float CollDis = (Rb.transform.position - Hit.point).magnitude - Radius;
            Vector3 OldMove = Movement * CollDis;
            SplitMoves.Add(OldMove);
            Movement = Movement * (1 - CollDis);
            Movement = Vector3.ProjectOnPlane(Movement, Hit.normal);

            PredPos += Movement;
            Range = Movement.magnitude + Radius + SkinDepth;
            WallNormal = Hit.normal;
        }
        return SplitMoves;
    }

    public Vector3 PredictDistance(Vector3 MovementInput)
    {
        Vector3 PredVel = MovementInput * Time.fixedDeltaTime;
        return PredVel * Time.fixedDeltaTime;
    }

    public void OnDrawGizmos()
    {
#if UNITY_EDITOR
        if (!EditorApplication.isPlaying)
            return;

        Vector3 A = GetGroundCastStart();
        Vector3 B = A + Vector3.down * 0.3f;
        Handles.DrawBezier(A, B, A, B, Color.red, null, 5);

        Vector3 Start = transform.position;
        for (int i = 0; i < MoveList.Count; i++)
        {
            if (MoveList[i].magnitude < 0.001f)
                continue;

            Vector3 End = PredictDistance(MoveList[i]) * 40;
            Handles.DrawBezier(Start, Start + End, Start, Start + End, Color.blue, null, 10);
            Start += End;
        }

        if (WallNormal.magnitude > 0.1f)
        {
            A = transform.position;
            B = A + WallNormal * 0.5f;
            Handles.DrawBezier(A, B, A, B, Color.yellow, null, 5);
        }
#endif
    }

    public void AddDebugText(string Text, bool bIsFixed = false)
    {
        TMPro.TextMeshProUGUI Target = bIsFixed ? FixedDebugText : DebugText;
        Target.text += Text;
    }

    public bool IsSprinting() { return bIsSprinting; }
    public bool IsJumping() { return bIsJumping; }
    public bool IsOnSlope() { return bIsOnSlope; }
    public bool IsGrounded() { return bIsGrounded; }
    public bool IsCrouching() { return bIsCrouching; }
    public bool IsHooking() { return bIsHooking; }
    public bool IsWallRunning() { return bIsWallrunning; }
    public Vector3 GetGroundNormal() { return GroundNormal; }
    public RaycastHit GetGroundHit() { return GroundHit; }
    public RaycastHit GetWallHit()
    {
        if (bIsWallLeft)
            return WallLeftHit;

        return WallRightHit;
    }
    public Rigidbody GetRigidbody() { return Rb; }
}
