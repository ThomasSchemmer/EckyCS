using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

/** Core component of the GAS, handles communication between different GAB's */
public class GameplayAbilitySystem : GameService
{
    public enum Type
    {
        DEFAULT,
        Player,
    }

    public SerializedDictionary<Type, GameplayAbilityBehaviour> Behaviours = new();

    private readonly SerializedDictionary<Type, UnityAction<GameplayAbilityBehaviour>> OnInitializedCallbacks = new();
    private readonly List<GameplayAbilityCue> LoadedCues = new();


    public void Update()
    {
        foreach (var Tuple in Behaviours)
        {
            Tuple.Value.Tick(Time.deltaTime);
        }
    }

    public void Register(GameplayAbilityBehaviour Behaviour, Type Type)
    {
        Behaviours.Add(Type, Behaviour);
        if (OnInitializedCallbacks.ContainsKey(Type))
        {
            OnInitializedCallbacks[Type].Invoke(Behaviour);
            OnInitializedCallbacks.Remove(Type);
        }
        _OnBehaviourRegistered?.Invoke(Behaviour);
    }

    public void DeRegister(GameplayAbilityBehaviour Behaviour, Type Type)
    {
        Behaviours.Remove(Type);
    }

    protected override void ResetInternal()
    {
        LoadCues();
        AttributeSet.Get().Reset();
    }


    private void LoadCues()
    {
        DestroyCues();
        foreach (var Obj in Resources.LoadAll(CuesPath))
        {
            if (Obj is not GameplayAbilityCue Cue)
                continue;

            var Instance = (GameplayAbilityCue)Cue.GetByInstancing(null);
            Instance.Init();
        }
    }

    private void DestroyCues()
    {
        for (int i = LoadedCues.Count - 1; i >= 0; i--)
        {
            Destroy(LoadedCues[i]);
        }
        LoadedCues.Clear();
    }

    protected override void StartServiceInternal()
    {
        LoadCues();
        _OnInit?.Invoke(this);
    }

    protected override void StopServiceInternal() {
        DestroyCues();
    }

    public bool TryApplyEffectTo(GameplayAbilityBehaviour Target, GameplayEffect Effect)
    {
        if (Target == null)
            return false;

        if (!Target.HasAllTags(Effect.ApplicationRequirementTags.IDs))
            return false;

        GameplayEffect Clone = Effect.GetByInstancing(Target) as GameplayEffect;

        Clone.SetTarget(Target);
        Target.AddEffect(Clone);
        return true;
    }

    public bool TryGiveAbilityTo(GameplayAbilityBehaviour Target, GameplayAbility Ability)
    {
        if (!Target.HasAbility(Ability))
        {
            Target.GrantAbility(Ability);
        }
        return true;
    }

    public bool TryActivateAbility(GameplayAbilityBehaviour Target, GameplayAbility Ability)
    {
        if (!TryGiveAbilityTo(Target, Ability))
            return false;

        if (!Ability.ShouldActivate())
            return false;

        Ability.Activate();
        return true;
    }

    public void RunAfterBehaviourRegistered(Type Type, UnityAction<GameplayAbilityBehaviour> Action)
    {
        if (Behaviours.ContainsKey(Type))
        {
            Action.Invoke(Behaviours[Type]);
        }
        else
        {
            OnInitializedCallbacks.Add(Type, Action);
        }
    }

    public delegate void OnBehaviourRegistered(GameplayAbilityBehaviour Behavior);
    public static OnBehaviourRegistered _OnBehaviourRegistered;

    private const string CuesPath = "GAS/Cues/";
}
