#include "GameplayEffectModifier.h"

#include "../GameplayAbilityComponent.h"
#include "../Attributes/Attribute.h"
#include "../Attributes/AttributeSet.h"
#include "../Attributes/AttributeType.h"

namespace GAS
{
    void GameplayEffectModifier::SetTarget(const shared_ptr<GameplayAbilityComponent>& NewTarget)
    {
        this->Target = NewTarget;
        TimeSinceLastActivatedS = 0;
        Attribute = Target->Attributes->Attributes[AttributeType];
    }
    void GameplayEffectModifier::Execute()
    {
        Attribute->AddModifier(shared_from_this());
    }
    
    void GameplayEffectModifier::Revert() const
    {
        Attribute->AddModifier(Invert());
    }
    
    void GameplayEffectModifier::Tick(float Delta)
    {
        TimeSinceLastActivatedS += Delta;
        if (TimeSinceLastActivatedS < PeriodS)
            return;

        TimeSinceLastActivatedS -= PeriodS;
        Attribute->AddModifier(shared_from_this());
    }
    
    string GameplayEffectModifier::GetDescription(int Multiplier /** = 1*/) const
    {
        switch (Operation)
        {
        case GameplayEffectModifierType::Add:
            return GetAddDescription(Multiplier);
        default:
            return GetNormalDescription();
        }
    }

    shared_ptr<GameplayEffectModifier> GameplayEffectModifier::Invert() const
    {
        if (Operation == GameplayEffectModifierType::Override)
            return nullptr;

        auto Copy = make_shared<GameplayEffectModifier>();
        Copy->PeriodS = this->PeriodS;
        Copy->AttributeType = this->AttributeType;
        Copy->SetTarget(this->Target);
        Copy->Operation = this->Operation;

        switch (this->Operation)
        {
        case GameplayEffectModifierType::Add:
        case GameplayEffectModifierType::Multiply:
            Copy->Value = -this->Value; break;
        case GameplayEffectModifierType::Override: break;
        }
        return Copy;
    }
    string GameplayEffectModifier::GetAddDescription(int Multiplier) const
    {
        return GetOperationDescription() + std::to_string(Value * Multiplier) +
            GetOperationPrepositionDescription() +
            GetAttributeDescription();
    }
    
    string GameplayEffectModifier::GetNormalDescription() const
    {
        return GetOperationDescription() +
            GetAttributeDescription() +
            GetOperationPrepositionDescription() +
            to_string(Value);
    }
    
    string GameplayEffectModifier::GetAttributeDescription() const
    {
        return AttributeTypeStrings[static_cast<int>(AttributeType)];
    }
    
    string GameplayEffectModifier::GetOperationDescription() const
    {
        switch (Operation)
        {
        case GameplayEffectModifierType::Add: return "Adds ";
        case GameplayEffectModifierType::Multiply: return Value > 0 ?  "Increases " : "Reduces ";
        case GameplayEffectModifierType::Override: return "Sets ";
        default: return "";
        }
    }
    
    string GameplayEffectModifier::GetOperationPrepositionDescription() const
    {
        switch (Operation)
        {
        case GameplayEffectModifierType::Add: return " to ";
        case GameplayEffectModifierType::Multiply: return " by ";
        case GameplayEffectModifierType::Override: return " to ";
        default: return "";
        }
    }
}
