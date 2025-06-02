using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using static GameplayEffect;

[CreateAssetMenu(fileName = "GameplayAbility", menuName = "ScriptableObjects/GameplayAbility", order = 4)]
public class GameplayAbility : ScriptableObject
{
    public enum State
    {
        Inactive,
        Activated,
        Committed,
        Ended
    }

    public State Status = State.Inactive;
    public KeyCode ActivationKey = KeyCode.Alpha1;
    public float Cooldown = -1;
    public GameplayAbilityBehaviour AssignedToBehaviour = null;
    public GameplayTagRegularContainer AbilityTags = new("Ability Tags");
    public GameplayTagRegularContainer ActivationRequiredTags = new("Activation required Tags");

    protected float CurrentCooldown;

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
            Status = State.Inactive;
        }
    }

    public virtual bool CanActivate()
    {
        bool bIsOffCooldown = CurrentCooldown == -1 || CurrentCooldown == 0;
        bool bIsInactive = Status == State.Inactive;
        bool bIsKeyDown = Input.GetKeyDown(ActivationKey);
        bool bHasTags = AssignedToBehaviour.HasTags(ActivationRequiredTags.IDs);
        return bIsOffCooldown && bIsInactive && bIsKeyDown && bHasTags;
    }

    public virtual void End()
    {
        Status = State.Ended;
        _OnEndAbility?.Invoke();
    }

    public delegate void OnEndAbility();
    public delegate void OnActivateAbility();
    public OnEndAbility _OnEndAbility;
    public OnActivateAbility _OnActivateAbility;
}
