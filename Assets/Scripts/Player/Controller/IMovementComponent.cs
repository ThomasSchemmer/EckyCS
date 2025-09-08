using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(PlayerController))]
public abstract class IMovementComponent : MonoBehaviour
{
    protected PlayerController Controller;

    public void Init()
    {
        Controller = GetComponent<PlayerController>();
        BaseInit();
    }
    public abstract void BaseInit();
    public abstract void UpdateState();
    public abstract void FixedUpdateMovement();
    public abstract bool ShouldProvideMovement();
    public abstract bool ShouldProvideJumpMovement();
    public abstract Vector3 GetMovement();
    public abstract Vector3 GetJumpMovement();
    public virtual void OnAfterJump() { }
    public abstract string GetName();
    public virtual void OnCollisionEnter(Collision collision) { }
}
