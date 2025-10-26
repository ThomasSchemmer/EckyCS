#include "GameplayAbility.h"
#include "../GameplayAbilityComponent.h"

namespace GAS
{
    
    void GameplayAbility::Tick(float Delta)
    {
        const float Max = std::max<float>(CurrentCooldown - Delta, 0.0f);
        CurrentCooldown = std::abs(-1 - CurrentCooldown) > 0.0001f ? Max : CurrentCooldown;

        const bool bCanReactivate = GameImports::Approximately(CurrentCooldown, 0) || GameImports::Approximately(CurrentCooldown, -1);
        if (State == GameplayAbilityState::Ended && bCanReactivate)
        {
            State = GameplayAbilityState::Granted;
        }
    }

    bool GameplayAbility::ShouldTick()
    {
        return State == GameplayAbilityState::Committed;
    }

    bool GameplayAbility::ShouldActivate()
    {
        bool bIsOffCooldown = GameImports::Approximately(CurrentCooldown, -1) || GameImports::Approximately(CurrentCooldown, 0);
        bool bIsGranted = State == GameplayAbilityState::Granted;
        bool bIsKeyDown = true;
        bool bHasTags = AssignedToComponent->HasAllTags(ActivationRequiredTags.IDs);
        return bIsOffCooldown && bIsGranted && bIsKeyDown && bHasTags;
    }

    bool GameplayAbility::ShouldDeactivate()
    {
        bool bIsActive = State >= GameplayAbilityState::Activated;
        bool bIsKeyDown = true;
        bool bHasTags = AssignedToComponent->HasAnyTags(DeActivationTriggerTags.IDs);

        return bIsActive && (bIsKeyDown || bHasTags);
    }
    
    void GameplayAbility::OnGranted()
    {
        State = GameplayAbilityState::Granted;
    }

    void GameplayAbility::OnRemoved()
    {
        State = GameplayAbilityState::Invalid;
    }

    void GameplayAbility::OnCommit()
    {
        State = GameplayAbilityState::Committed;    
    }

    void GameplayAbility::OnActivate()
    {
        State = GameplayAbilityState::Activated;
        CurrentCooldown = Cooldown;
        OnActivateAbility.ForEach(shared_from_this());
        OnCommit();
    }

    void GameplayAbility::OnDeactivate()
    {
        State = GameplayAbilityState::Granted;
        CurrentCooldown = Cooldown;
        OnEndAbility.ForEach(shared_from_this());
    }
}