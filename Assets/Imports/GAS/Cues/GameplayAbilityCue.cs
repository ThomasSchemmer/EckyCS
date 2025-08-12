using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Cue", menuName = "ScriptableObjects/GAS/Cue", order = 4)]
public abstract class GameplayAbilityCue : GameplayAbilityInstancingPolicy
{
    public GameplayTagRegularContainer AssignedTags;

    public void Tick(GameplayAbilityBehaviour Owner, float Delta)
    {

    }

    public void OnEnableCue()
    {

    }

    public void OnDisableCue()
    {

    }
}
