using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameplayAbilityBehaviour : MonoBehaviour
{
    public GameplayAbilitySystem.Type Type = GameplayAbilitySystem.Type.DEFAULT;
    public AttributeSet Attributes;
    public List<GameplayAbility> GrantedOnStart = new();
    private List<GameplayEffect> ActiveEffects = new();
    private List<GameplayEffect> MarkedForRemovalEffects = new();
    private List<GameplayAbility> GrantedAbilities = new();
    private GameplayTagMask GameplayTagMask = new();

    private bool bIsInitialized = false;

    private List<Tuple<GameplayEffect, bool>> EffectsToHandle = new();

    void Start()
    {
        Game.RunAfterServiceInit((GameplayAbilitySystem System) =>
        {
            Attributes.Initialize();
            bIsInitialized = true;
            HandleDelayedEffects();
            HandleOnStartAbilities(System);

            System.Register(this, Type);
        });
    }

    public void Tick(float Delta)
    {
        if (!bIsInitialized)
            return;

        TickAbilities(Delta);
        TickEffects(Delta);

        // now attributes can be calculated
        Attributes.Tick();
    }

    private void TickAbilities(float Delta)
    {
        foreach (var Ability in GrantedAbilities)
        {
            if (Ability.CanActivate())
            {
                Ability.Activate();
            }
            Ability.Tick(Delta);
        }
    }

    private void TickEffects(float Delta)
    {
        // Effects can register themselves on attributes
        foreach (GameplayEffect ActiveEffect in ActiveEffects)
        {
            bool bIsExpired = ActiveEffect.IsExpired(Delta);
            bool bHasTags = HasTags(ActiveEffect.OngoingRequirementTags.IDs);
            if (!bHasTags || bIsExpired)
            {
                MarkedForRemovalEffects.Add(ActiveEffect);
                continue;
            }

            ActiveEffect.Tick(Delta);
        }
        foreach (GameplayEffect ToRemoveEffect in MarkedForRemovalEffects)
        {
            RemoveEffectByTags(ToRemoveEffect.GrantedTags);
        }
        MarkedForRemovalEffects.Clear();
    }

    private void HandleDelayedEffects()
    {
        if (!bIsInitialized)
        {
            throw new System.Exception("Cannot handle delayed effects if not fully initialized!");
        }

        foreach (var Tuple in EffectsToHandle)
        {
            if (Tuple.Value)
            {
                AddEffect(Tuple.Key);
            }
            else
            {
                RemoveEffectByTags(Tuple.Key.GrantedTags);
            }
        }
        EffectsToHandle.Clear();
    }

    private void HandleOnStartAbilities(GameplayAbilitySystem System)
    {
        foreach (var Ability in GrantedOnStart)
        {
            System.TryGiveAbilityTo(this, Instantiate(Ability));
        }
    }

    public void AddTagByID(Guid ID) {
        GameplayTagMask.Set(ID);

        _OnTagsChanged?.Invoke();
        _OnTagAdded?.Invoke(ID);
    }

    public void AddTagsByID(List<Guid> IDs)
    {
        foreach (Guid ID in IDs)
        {
            AddTagByID(ID);
        }
    }

    public void RemoveTag(Guid ID)
    {
        GameplayTagMask.Remove(ID);

        _OnTagsChanged?.Invoke();
        _OnTagAdded?.Invoke(ID);
    }

    public void RemoveTags(List<Guid> IDs)
    {
        foreach (Guid ID in IDs)
        {
            RemoveTag(ID);
        }
    }

    public bool HasTag(Guid Tag)
    {
        return GameplayTagMask.HasID(Tag);
    }

    public bool HasTags(List<Guid> IDs)
    {
        foreach (var ID in IDs)
        {
            if (!HasTag(ID))
                return false;
        }
        return true;
    }

    //don't call this directly, call @GAS.TryApplyEffectTo instead!
    public void AddEffect(GameplayEffect Effect) { 
        if (!bIsInitialized)
        {
            EffectsToHandle.Add(new(Effect, true));
            return;
        }

        ActiveEffects.Add(Effect);
        AddTagsByID(Effect.GrantedTags.IDs);
        Effect.Execute();

        if (Effect.GrantedAbility == null || Effect.DurationPolicy == GameplayEffect.Duration.Instant)
            return;

        GrantAbility(Effect.GrantedAbility);
    }

    public List<GameplayEffect> GetActiveEffects()
    {
        return ActiveEffects;
    }

    public bool TryGetAnyActiveEffectByTags(List<Guid> Tags, out List<GameplayEffect> FoundEffects)
    {
        FoundEffects = new();
        foreach (var Tag in Tags)
        {
            if (!TryGetAnyActiveEffectByTags(Tag, out var Effect))
                continue;

            FoundEffects.Add(Effect);
        }
        return Tags.Count > 0;
    }

    public bool TryGetAnyActiveEffectByTags(Guid Tag, out GameplayEffect FoundEffect)
    {
        FoundEffect = default;
        if (!GameplayTagMask.HasID(Tag))
            return false;

        foreach(var Effect in ActiveEffects)
        {
            if (!Effect.GrantedTags.IDs.Contains(Tag))
                continue;

            FoundEffect = Effect;
            return true;
        }
        return false;
    }

    public void RemoveEffectByTags(GameplayTagRegularContainer Tags)
    {
        if (!bIsInitialized)
            return;

        foreach(var Tag in Tags.IDs)
        {
            RemoveEffectByTag(Tag);
        }
    }

    public void RemoveEffectByTag(Guid Tag)
    {
        if (!bIsInitialized)
            return;

        if (!TryGetAnyActiveEffectByTags(Tag, out var Effect))
            return;

        ActiveEffects.Remove(Effect);
        RemoveTags(Effect.GrantedTags.IDs);
        Effect.Deactivate();

        if (Effect.GrantedAbility == null || Effect.DurationPolicy == GameplayEffect.Duration.Instant)
            return;

        GrantedAbilities.Remove(Effect.GrantedAbility);
    }

    public void RemoveEffect(GameplayEffect Effect)
    {
        RemoveEffectByTags(Effect.GrantedTags);
    }

    public void GrantAbility(GameplayAbility Ability)
    {
        GrantedAbilities.Add(Ability);
        Ability.AssignedToBehaviour = this;
    }

    public bool HasAbility(GameplayAbility Ability)
    {
        return GrantedAbilities.Contains(Ability);
    }

    public void RemoveAbility(GameplayAbility Ability)
    {
        GrantedAbilities.Remove(Ability);
    }

    public List<GameplayAbility> GetGrantedAbilities()
    {
        return GrantedAbilities;
    }

    public delegate void OnTagsChanged();
    public delegate void OnTagAdded(Guid Tag);
    public delegate void OnTagRemoved(Guid Tag);
    public OnTagsChanged _OnTagsChanged;
    public OnTagAdded _OnTagAdded;
    public OnTagRemoved _OnTagRemoved;
}
