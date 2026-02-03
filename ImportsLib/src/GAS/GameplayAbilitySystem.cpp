#include "GameplayAbilitySystem.h"

#include "GameplayAbilityComponent.h"
#include "Attributes/AttributeSet.h"
#include "Effects/GameplayEffect.h"

namespace GAS
{
    GameplayAbilitySystem::GameplayAbilitySystem()
    {
        Type = GameServiceType::GameplayAbilitySystem;
    }

    void GameplayAbilitySystem::Update(float Delta)
    {
        for (const auto& Pair : Components)
        {
            for (auto& Component : Pair.second)
            {
                Component->Tick(Delta);
            }
        }
    }
    
    void GameplayAbilitySystem::Register(const shared_ptr<GameplayAbilityComponent>& Component, GameplayAbilityComponentType ComponentType)
    {
        if (Components.contains(ComponentType))
        {
            Components.insert({ComponentType, {}});
        }
        Components[ComponentType].emplace_back(Component);
        OnComponentRegistered.ForEach(Component);
    }
    
    void GameplayAbilitySystem::DeRegister(const shared_ptr<GameplayAbilityComponent>& Component, GameplayAbilityComponentType ComponentType)
    {
        if (!Components.contains(ComponentType))
            return;

        std::erase(Components[ComponentType], Component); 
    }

    bool GameplayAbilitySystem::TryApplyEffectTo(const shared_ptr<GameplayAbilityComponent>& Target, const shared_ptr<GameplayEffect>& Effect)
    {
        if (!Target->HasAllTags(Effect->ApplicationRequiredTags.IDs))
            return false;

        auto Clone = Effect->GetByInstancing<GameplayEffect>(Target);
        Clone->SetTarget(Target);
        Target->AddEffect(Clone);
        return true;
    }
    
    bool GameplayAbilitySystem::TryGiveAbilityTo(const shared_ptr<GameplayAbilityComponent>& Target, const shared_ptr<GameplayAbility>& Ability)
    {
        if (!Target->HasAbility(Ability))
        {
            Target->GrantAbility(Ability);
        }
        return true;
    }
    
    bool GameplayAbilitySystem::TryActivateAbility(const shared_ptr<GameplayAbilityComponent>& Target, const shared_ptr<GameplayAbility>& Ability)
    {
        if (!TryGiveAbilityTo(Target, Ability))
            return false;

        if (!Ability->ShouldActivate())
            return false;

        Ability->OnActivate();
        return true;
    }
    
    void GameplayAbilitySystem::StartServiceInternal()
    {
        OnInit.ForEach(this->Type);
    }
    
    void GameplayAbilitySystem::StopServiceInternal() {}
    
    void GameplayAbilitySystem::ResetServiceInternal()
    {
        AttributeSet::Get().Reset();
    }
}
