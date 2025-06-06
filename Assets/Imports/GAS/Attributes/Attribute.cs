using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
/** 
 * Represents a runtime parameter that affects the players stats. Contained in an @AttributeSet
 * Can have modifier applied to it via @GameplayEffect
 */
public class Attribute
{
    public AttributeType Type;
    public float BaseValue = 0;
    public float CurrentValue = 0;

    // called in the editor - do not delete!
    public Attribute()
    {
        Modifiers = new();
        Initialize();
    }

    public Attribute(AttributeType Type)
    {
        this.Type = Type;
        Modifiers = new();
        Initialize();
    }

    public void Initialize()
    {
        Reset();
    }

    public void Reset()
    {
        CurrentValue = BaseValue;
        ResetModifiers();
        _OnAttributeChanged?.Invoke();
    }

    private void ResetModifiers()
    {
        Modifiers.Remove(GameplayEffectModifier.Type.Add);
        Modifiers.Remove(GameplayEffectModifier.Type.Multiply);
        Modifiers.Remove(GameplayEffectModifier.Type.Override);
        Modifiers.Add(GameplayEffectModifier.Type.Add, new());
        Modifiers.Add(GameplayEffectModifier.Type.Multiply, new());
        Modifiers.Add(GameplayEffectModifier.Type.Override, new());
    }

    public void Tick()
    {
        float MultiplyValue = 1 + Multiply;
        CurrentValue = (BaseValue + Add) * MultiplyValue;
        if (!Mathf.Approximately(Override, 0))
        {
            CurrentValue = Override;
        }

        if (!Mathf.Approximately(BaseValue, CurrentValue))
        {
            _OnAttributeChanged?.Invoke();
        }
    }

    public void AddModifier(GameplayEffectModifier Modifier)
    {
        if (!Modifiers.TryGetValue(Modifier.Operation, out List<GameplayEffectModifier> TargetList))
            return;

        TargetList.Add(Modifier);
        Tick();
    }

    public void RemoveModifier(GameplayEffectModifier Modifier)
    {
        if (!Modifiers.TryGetValue(Modifier.Operation, out List<GameplayEffectModifier> TargetList))
            return;

        TargetList.Remove(Modifier);
    }

    private float GetModifiedValueFor(GameplayEffectModifier.Type Operation)
    {
        if (!Modifiers.TryGetValue(Operation, out List<GameplayEffectModifier> TargetList))
            return 0;

        float Result = 0;
        foreach (GameplayEffectModifier Modifier in TargetList)
        {
            Result += Modifier.Value;
        }

        return Result;
    }

    private float Add
    {
        get { return GetModifiedValueFor(GameplayEffectModifier.Type.Add); }
    }

    private float Multiply
    {
        get { return GetModifiedValueFor(GameplayEffectModifier.Type.Multiply); }
    }

    private float Override
    {
        get { return GetModifiedValueFor(GameplayEffectModifier.Type.Override); }
    }

    Dictionary<GameplayEffectModifier.Type, List<GameplayEffectModifier>> Modifiers;

    public delegate void OnAttributeChanged();
    public OnAttributeChanged _OnAttributeChanged;
}
