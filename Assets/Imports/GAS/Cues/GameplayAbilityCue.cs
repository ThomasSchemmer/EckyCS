using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor.Playables;
using UnityEngine;

[CreateAssetMenu(fileName = "Cue", menuName = "ScriptableObjects/GAS/Cue", order = 4)]
public abstract class GameplayAbilityCue : GameplayAbilityInstancingPolicy
{
    [SerializeField]
    public GameplayAbilitySystem.Type BehaviourType;
    [SerializeField]
    public AbilityType AbilityType;

    protected GameplayAbilityBehaviour AssignedToBehaviour;
    protected GameplayAbility AssignedToAbility;

    public abstract void OnBeforeAbilityTick();
    public abstract void OnAfterAbilityTick();
    protected abstract void EnableCue();
    protected abstract void DisableCue();

    public void Init()
    {
        Game.RunAfterServiceInit((GameplayAbilitySystem GAS) =>
        {
            GAS.RunAfterBehaviourRegistered(BehaviourType, (Target) =>
            {
                AssignedToBehaviour = Target;
                Target.RunAfterAbilityGranted(AbilityType, (Ability) =>
                {
                    InitInternal(Ability);
                });
            });
        });
    }

    protected virtual void InitInternal(GameplayAbility Ability)
    {
        AssignedToAbility = Ability;
        AssignedToAbility.AssignedCues.Add(this);
        AssignedToAbility._OnActivateAbility += EnableCue;
        AssignedToAbility._OnEndAbility += DisableCue;
    }

    public void OnDestroy()
    {
        if (AssignedToAbility)
        {
            AssignedToAbility.AssignedCues.Remove(this);
            AssignedToAbility._OnActivateAbility -= EnableCue;
            AssignedToAbility._OnEndAbility -= DisableCue;
        }
    }
}
