#include "GameplayEffect.h"

#include "GameplayEffectModifier.h"

namespace GAS
{
    void GameplayEffect::SetTarget(shared_ptr<GameplayAbilityComponent> NewTarget)
    {
        Target = NewTarget;
        bIsDeactivated = false;
        Runtime = 0;
        for (auto& Modifier : Modifiers)
        {
            Modifier->SetTarget(Target);
        }
    }
    
    void GameplayEffect::Execute() const
    {
        if (DurationType != GameplayEffectDuration::Instant)
            return;

        for (auto& Modifier : Modifiers)
        {
            Modifier->Execute();
        }

        Target->RemoveTags(RemoveTags.IDs);
    }
    
    void GameplayEffect::Deactivate()
    {
        if (DurationType != GameplayEffectDuration::Instant)
            return;

        for (auto& Modifier : Modifiers)
        {
            Modifier->Revert();
        }
        bIsDeactivated = true;
    }
    
    void GameplayEffect::Tick(float Delta) const
    {
        if (DurationType == GameplayEffectDuration::Instant)
            return;

        for (auto& Modifier : Modifiers)
        {
            Modifier->Tick(Delta);
        }
    }
    
    bool GameplayEffect::IsExpired(float Delta)
    {
        if (bIsDeactivated)
            return true;

        if (DurationType == GameplayEffectDuration::Instant)
            return false;

        Runtime += Delta;
        return Runtime > DurationLength; 
    }

    std::vector<shared_ptr<GameplayEffectModifier>> GameplayEffect::GetModifiersByOperation(GameplayEffectModifierType OperationType) const
    {
        vector<shared_ptr<GameplayEffectModifier>> SelectedModifiers;
        for (auto& Modifier : Modifiers)
        {
            if (Modifier->Operation != OperationType)
                continue;

            SelectedModifiers.push_back(Modifier);
        }
        return SelectedModifiers;
    }
    
    std::string GameplayEffect::GetEffectDescription() const
    {
        string Result;
        const int Size = static_cast<int>(Modifiers.size());
        for (int i = 0; i < Size; ++i)
        {
            Result += Modifiers[i]->GetDescription();
            if (i == Size - 1)
                continue;
            
            Result += "\n";
        }
        return Result;
    }

    bool GameplayEffect::IsSelfOnActor(const shared_ptr<GameplayAbilityComponent>& Actor)
    {
        //todo: might not work if comp by ref!
        for (auto& Effect : Actor->GetActiveEffects())
        {
            if (typeid(Effect.get()) != typeid(this))
                continue;
            return true;
        }
        return false;
    }
}
