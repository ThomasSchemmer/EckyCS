#include "GameplayAbilityComponent.h"

#include "GameplayAbilitySystem.h"
#include "../GameService/Game.h"
#include "../GameService/GameServiceDelegate.h"
#include "Abilities/GameplayAbility.h"
#include "Attributes/AttributeSet.h"
#include "Effects/GameplayEffect.h"

namespace GAS
{
    using namespace GameImports;
    
    GameplayAbilityComponent::GameplayAbilityComponent()
    {
    }

    GameplayAbilityComponent::~GameplayAbilityComponent()
    {
        TemplatedDelegate<GameplayAbilitySystem>::RunAfterServiceInit([this](const shared_ptr<GameplayAbilitySystem>& System)
        {
            System->DeRegister(shared_from_this(), Type);
        },
        GameServiceType::INVALID,
        GameServiceType::GameplayAbilitySystem
        );
    }

    void GameplayAbilityComponent::Init()
    {
        TemplatedDelegate<GameplayAbilitySystem>::RunAfterServiceInit([this](const shared_ptr<GameplayAbilitySystem>& System)
        {
            Attributes->Initialize();
            bIsInitialized = true;
            HandleDelayedEffects();
            HandleOnStartAbilities();

            System->Register(shared_from_this(), Type);
        },
        GameServiceType::INVALID,
        GameServiceType::GameplayAbilitySystem
        );
    }

    void GameplayAbilityComponent::Tick(float Delta)
    {
        if (!bIsInitialized)
            return;
        
        TickAbilities(Delta);
        TickEffects(Delta);

        //now attributes can be calculated
        Attributes->Tick();
    }
    
    void GameplayAbilityComponent::TickAbilities(float Delta) const
    {
        for (auto& Tuple : GrantedAbilities)
        {
            auto& Ability = Tuple.second;
            if (Ability->ShouldActivate())
            {
                Ability->OnActivate();
            }else if (Ability->ShouldDeactivate())
            {
                Ability->OnDeactivate();
            }
            if (Ability->ShouldTick())
            {
                Ability->Tick(Delta);
            }
        }
    }
    
    void GameplayAbilityComponent::TickEffects(float Delta)
    {
        // Effects can register themselves on attributes
        for (const auto& ActiveEffect : ActiveEffects)
        {
            const bool bIsExpired = ActiveEffect->IsExpired(Delta);
            const bool bHasTags = HasAllTags(ActiveEffect->OngoingRequirementTags.IDs);
            if (!bHasTags || bIsExpired)
            {
                MarkedForRemovalEffects.emplace_back(ActiveEffect);
                continue;
            }

            ActiveEffect->Tick(Delta);
        }
        for (const auto& ToRemoveEffect : MarkedForRemovalEffects)
        {
            RemoveEffect(ToRemoveEffect);
        }
        MarkedForRemovalEffects.clear();
    }
    
    void GameplayAbilityComponent::AddTag(const string& Tag)
    {
        ActiveTags.Set(Tag);
        OnTagsChanged.ForEach(ActiveTags);
        OnTagAdded.ForEach(Tag);
    }
    
    void GameplayAbilityComponent::RemoveTag(const string& Tag)
    {
        ActiveTags.Remove(Tag);
        OnTagsChanged.ForEach(ActiveTags);
        OnTagRemoved.ForEach(Tag);
    }
    
    void GameplayAbilityComponent::AddTags(const vector<string>& Tags)
    {
        for (const auto& Tag : Tags)
        {
            AddTag(Tag);
        }
    }
    
    void GameplayAbilityComponent::RemoveTags(const vector<string>& Tags)
    {
        for (const auto& Tag : Tags)
        {
            RemoveTag(Tag);
        }
    }
    bool GameplayAbilityComponent::HasTag(const string& Tag)
    {
        return ActiveTags.HasID(Tag);
    }
    
    bool GameplayAbilityComponent::HasAllTags(const vector<string>& Tags)
    {
        for (const auto& Tag : Tags)
        {
            if (!HasTag(Tag))
                return false;
        }
        return true;
    }
    
    bool GameplayAbilityComponent::HasAnyTags(const vector<string>& Tags)
    {
        for (const auto& Tag : Tags)
        {
            if (HasTag(Tag))
                return true;
        }
        return false;
    }

    void GameplayAbilityComponent::AddEffect(shared_ptr<GameplayEffect> Effect)
    {
        if (!bIsInitialized)
        {
            EffectsToHandle.emplace(Effect, true);
            return;
        }

        ActiveEffects.emplace_back(Effect);
        AddTags(Effect->GrantedTags.IDs);
        Effect->Execute();
        
        if (Effect->GrantedAbility == nullptr || Effect->DurationType == GameplayEffectDuration::Instant)
            return;

        GrantAbility(Effect->GrantedAbility);
    }
    
    vector<shared_ptr<GameplayEffect>> GameplayAbilityComponent::GetActiveEffects()
    {
        return ActiveEffects;
    }

    bool GameplayAbilityComponent::TryGetAnyActiveEffectsByTags(const vector<string>& Tags, vector<shared_ptr<GameplayEffect>>& FoundEffects)
    {
        for (const auto& Tag : Tags)
        {
            shared_ptr<GameplayEffect> Effect;
            if (!TryGetAnyActiveEffectByTag(Tag, Effect))
                continue;
            
            FoundEffects.emplace_back(Effect);
        }
        return !FoundEffects.empty();
    }

    bool GameplayAbilityComponent::TryGetAnyActiveEffectByTag(const string& Tag, shared_ptr<GameplayEffect>& FoundEffect)
    {
        if (!ActiveTags.HasID(Tag))
            return false;

        for (auto& Effect : ActiveEffects)
        {
            auto& List = Effect->GrantedTags.IDs;
            if (ranges::find(List, Tag) == List.end())
                continue;

            FoundEffect = Effect;
            return true;
        }
        return false;
    }
    
    void GameplayAbilityComponent::RemoveEffectByTag(const string& Tag)
    {
        if (!bIsInitialized)
            throw std::exception("ERROR::GAS::COMPONENT_NOT_INITIALIZED");

        shared_ptr<GameplayEffect> Effect;
        if (!TryGetAnyActiveEffectByTag(Tag, Effect))
            return;

        std::erase(ActiveEffects, Effect);
        RemoveTags(Effect->GrantedTags.IDs);
        Effect->Deactivate();

        if (Effect->GrantedAbility == nullptr || Effect->DurationType == GameplayEffectDuration::Instant)
            return;

        RemoveAbility(Effect->GrantedAbility);
    }
    
    void GameplayAbilityComponent::RemoveEffect(const shared_ptr<GameplayEffect>& Effect)
    {
        for (auto& Tag : Effect->GrantedTags.IDs)
        {
            RemoveEffectByTag(Tag);
        }
    }

    void GameplayAbilityComponent::GrantAbility(const shared_ptr<GameplayAbility>& Ability)
    {
        GrantedAbilities.emplace(Ability->Type, Ability);
        Ability->AssignedToComponent = shared_from_this();
        OnAbilityGranted.ForEach(Ability);

        if (OnAbilityGrantedCallbacks.contains(Ability->Type))
        {
            OnAbilityGrantedCallbacks[Ability->Type].ForEach(Ability);
            OnAbilityGrantedCallbacks.erase(Ability->Type);
        }
        Ability->OnGranted();
    }
    
    void GameplayAbilityComponent::RemoveAbility(const shared_ptr<GameplayAbility>& Ability)
    {
        GrantedAbilities.erase(Ability->Type);
        OnAbilityRemoved.ForEach(Ability);
        Ability->OnRemoved();
    }
    
    vector<shared_ptr<GameplayAbility>> GameplayAbilityComponent::GetGrantedAbilities() const
    {
        vector<shared_ptr<GameplayAbility>> List;
        for (const auto& Tuple : GrantedAbilities)
        {
            List.emplace_back(Tuple.second);
        }
        return List;
    }
    
    bool GameplayAbilityComponent::HasAbility(shared_ptr<GameplayAbility> Ability) const
    {
        return GrantedAbilities.contains(Ability->Type);
    }
    
    void GameplayAbilityComponent::RunAfterAbilityGranted(AbilityType AbilityType, GameImports::Action<shared_ptr<GameplayAbility>>& Callback)
    {
        if (GrantedAbilities.contains(AbilityType))
        {
            Callback(GrantedAbilities[AbilityType]);
        }else
        {
            OnAbilityGrantedCallbacks[AbilityType].Add(AbilityType, Callback);
        }
    }
    
    void GameplayAbilityComponent::HandleDelayedEffects()
    {
        if (!bIsInitialized)
            throw std::exception("ERROR:GAS::COMPONENT_NOT_INITIALIZED");

        for (auto& Tuple : EffectsToHandle)
        {
            auto& Effect = std::get<0>(Tuple);
            if (std::get<1>(Tuple))
            {
                AddEffect(Effect);
            }else
            {
                RemoveEffect(Effect);
            }
        }
    }
    
    void GameplayAbilityComponent::HandleOnStartAbilities()
    {
        for (auto& Ability : GrantedOnStart)
        {
            GameplayAbilitySystem::TryGiveAbilityTo(shared_from_this(), Ability);
        }
    }
}
