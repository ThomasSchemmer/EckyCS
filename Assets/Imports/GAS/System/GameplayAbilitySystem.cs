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
    private SerializedDictionary<Type, UnityAction<GameplayAbilityBehaviour>> OnInitializedCallbacks = new();

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
            OnInitializedCallbacks[Type]?.Invoke(Behaviour);
        }
        _OnBehaviourRegistered?.Invoke(Behaviour);
    }

    protected override void ResetInternal()
    {
        
        AttributeSet.Get().Reset();
    }

    protected override void StartServiceInternal()
    {
        _OnInit?.Invoke(this);
    }

    protected override void StopServiceInternal() {}

    public bool TryApplyEffectTo(GameplayAbilityBehaviour Target, GameplayEffect Effect)
    {
        if (Target == null)
            return false;

        if (!Target.HasTags(Effect.ApplicationRequirementTags.IDs))
            return false;

        GameplayEffect Clone = Effect.GetByInstancing(Target);

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

        if (!Ability.CanActivate())
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
}
