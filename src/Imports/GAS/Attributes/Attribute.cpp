#include "Attribute.h"

namespace GAS
{
    Attribute::Attribute()
    {
        Initialize();
    }
    
    Attribute::Attribute(AttributeType NewType)
    {
        this->Type = NewType;
        Initialize();
    }

    void Attribute::Initialize()
    {
        Reset();
    }
    
    void Attribute::Reset()
    {
        CurrentValue = BaseValue;
        ResetModifiers();
        OnAttributeChanged.ForEach(Type);
    }
    
    void Attribute::Tick()
    {
        const float Multiply = 1 + GetModifiedValueFor(GameplayEffectModifierType::Multiply);
        const float Add = GetModifiedValueFor(GameplayEffectModifierType::Add);
        const float Override = GetModifiedValueFor(GameplayEffectModifierType::Override);
        
        CurrentValue = (BaseValue + Add) * Multiply;
        if (!GameImports::Approximately(Override, 0))
        {
            CurrentValue = Override;
        }
        if (!GameImports::Approximately(BaseValue, CurrentValue))
        {
            OnAttributeChanged.ForEach(Type);
        }
    }
    
    void Attribute::AddModifier(const shared_ptr<GameplayEffectModifier>& Modifier)
    {
        Modifiers[Modifier->Operation].emplace_back(Modifier);
        Tick();
    }
    
    void Attribute::RemoveModifier(shared_ptr<GameplayEffectModifier>& Modifier)
    {
        std::erase(Modifiers[Modifier->Operation], Modifier);
    }

    void Attribute::ResetModifiers()
    {
        Modifiers.erase(GameplayEffectModifierType::Add);
        Modifiers.erase(GameplayEffectModifierType::Multiply);
        Modifiers.erase(GameplayEffectModifierType::Override);
        Modifiers[GameplayEffectModifierType::Add] = {};
        Modifiers[GameplayEffectModifierType::Multiply] = {};
        Modifiers[GameplayEffectModifierType::Override] = {};
    }
    
    float Attribute::GetModifiedValueFor(GameplayEffectModifierType TargetType)
    {
        float Result = 0;
        for (const auto& Modifier : Modifiers[TargetType])
        {
            Result += Modifier->Value;
        }
        return Result;
    }
}