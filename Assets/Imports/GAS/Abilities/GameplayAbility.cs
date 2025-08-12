using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using static GameplayEffect;

[CreateAssetMenu(fileName = "GameplayAbility", menuName = "ScriptableObjects/GAS/Ability", order = 4)]
public class GameplayAbility : ScriptableObject
{
    public enum State
    {
        Invalid,    // nothing has happened to this ability, it exists only as a definition
        Granted,    // has been granted to a behaviour, but not yet used
        Activated,  // has been started, might not have been fully approved
        Committed,  // All checks have been cleared, actively running, will stay until timeout etc
        Ended       // Has been executed, cannot be used again
    }

    public AbilityType Type;
    public State Status = State.Invalid;
    public KeyCode ActivationKey = KeyCode.Alpha1;
    public float Cooldown = -1;
    public GameplayAbilityBehaviour AssignedToBehaviour = null;
    public GameplayTagRegularContainer AbilityTags = new("Ability Tags");
    public GameplayTagRegularContainer ActivationRequiredTags = new("Activation required Tags");

    public ActionList<GameplayAbilityBehaviour> OnTargetHit = new();

    protected float CurrentCooldown;

    public virtual void OnGranted()
    {
        Status = State.Granted;
    }

    public virtual void OnRemoved()
    {
        Status = State.Invalid;
    }

    public virtual void Activate()
    {
        Status = State.Activated;
        CurrentCooldown = Cooldown;
        _OnActivateAbility?.Invoke();
        Commit();
    }

    public virtual void Commit()
    {
        Status = State.Committed;
    }

    public virtual void Tick(float Delta)
    {
        CurrentCooldown = CurrentCooldown != -1 ? Mathf.Max(CurrentCooldown - Delta, 0) : CurrentCooldown;

        bool bCanReactivate = CurrentCooldown == 0 || CurrentCooldown == -1;
        if (Status == State.Ended && bCanReactivate)
        {
            Status = State.Granted;
        }
    }

    public virtual bool ShouldTick()
    {
        return Status == State.Committed;
    }

    public virtual bool CanActivate()
    {
        bool bIsOffCooldown = CurrentCooldown == -1 || CurrentCooldown == 0;
        bool bIsGranted = Status == State.Granted;
        bool bIsKeyDown = Input.GetKeyDown(ActivationKey);
        bool bHasTags = AssignedToBehaviour.HasTags(ActivationRequiredTags.IDs);
        return bIsOffCooldown && bIsGranted && bIsKeyDown && bHasTags;
    }

    public virtual void End()
    {
        Status = State.Ended;
        _OnEndAbility?.Invoke();
    }
    public float GetCooldownCutoff()
    {
        return 1 - CurrentCooldown;
    }
    public void OnAbilityHit(GameplayAbilityBehaviour Target)
    {
        OnTargetHit.ForEach(_ => _?.Invoke(Target));
    }

    public delegate void OnEndAbility();
    public delegate void OnActivateAbility();
    public OnEndAbility _OnEndAbility;
    public OnActivateAbility _OnActivateAbility;
}
