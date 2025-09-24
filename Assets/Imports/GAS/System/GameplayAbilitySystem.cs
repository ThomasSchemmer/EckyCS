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

    public SerializedDictionary<Type, List<GameplayAbilityBehaviour>> Behaviours = new();

    private readonly SerializedDictionary<
        Type, 
        List<(UnityAction<GameplayAbilityBehaviour>, bool)>
    > OnBehaviourRegisteredCallbacks = new();
    private readonly List<GameplayAbilityCue> LoadedCues = new();


    public void Update()
    {
        foreach (var BehList in Behaviours)
        {
            foreach (var Behaviour in BehList.Value)
            {
                Behaviour.Tick(Time.deltaTime);
            }
        }
    }

    public void Register(GameplayAbilityBehaviour Behaviour, Type Type)
    {
        if (!Behaviours.ContainsKey(Type))
        {
            Behaviours.Add(Type, new());
        }
        Behaviours[Type].Add(Behaviour);
        TriggerBehaviourCallbacks(Behaviour, Type);
        _OnBehaviourRegistered?.Invoke(Behaviour);
    }

    private void TriggerBehaviourCallbacks(GameplayAbilityBehaviour Behaviour, Type Type)
    {
        if (!OnBehaviourRegisteredCallbacks.ContainsKey(Type))
            return;

        for (int i = OnBehaviourRegisteredCallbacks[Type].Count - 1; i >= 0; i--)
        {
            var Tuple = OnBehaviourRegisteredCallbacks[Type][i];
            Tuple.Item1.Invoke(Behaviour);
            if (!Tuple.Item2)
                continue;

            RemoveBehaviourRegisteredCallback(Type, OnBehaviourRegisteredCallbacks[Type][i].Item1);
        }
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

    public void RunAfterBehaviourRegistered(Type Type, UnityAction<GameplayAbilityBehaviour> Action, bool bRemoveAfterRun)
    {
        if (!OnBehaviourRegisteredCallbacks.ContainsKey(Type))
        {
            OnBehaviourRegisteredCallbacks.Add(Type, new());
        }
        OnBehaviourRegisteredCallbacks[Type].Add((Action, bRemoveAfterRun));
        
        if (!Behaviours.ContainsKey(Type))
            return;
        
        foreach (var Behaviour in Behaviours[Type])
        {
            TriggerBehaviourCallbacks(Behaviour, Type);
        }
    }

    public void RemoveBehaviourRegisteredCallback(Type Type, UnityAction<GameplayAbilityBehaviour> Action)
    {
        if (!OnBehaviourRegisteredCallbacks.ContainsKey(Type))
            return;

        var List = OnBehaviourRegisteredCallbacks[Type];
        for (int i = List.Count - 1; i >= 0; i--)
        {
            if (!List[i].Item1.Equals(Action))
                continue;

            List.RemoveAt(i);
        }
    }

    public delegate void OnBehaviourRegistered(GameplayAbilityBehaviour Behavior);
    public static OnBehaviourRegistered _OnBehaviourRegistered;

    private const string CuesPath = "GAS/Cues/";
}
