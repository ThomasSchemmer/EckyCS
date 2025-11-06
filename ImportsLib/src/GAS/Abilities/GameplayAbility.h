#pragma once
#include <cstdint>
#include <memory>

#include "AbilityType.h"
#include "../../Util/ActionMap.h"
#include "../Tags/GameplayTagContainer.h"

namespace GAS
{
    enum class GameplayAbilityComponentType : uint8_t;
    using namespace GameImports;
    using namespace std;
    
    enum class AbilityType : uint8_t;
    class GameplayAbilityComponent;
    
    enum class GameplayAbilityState : uint8_t
    {
        Invalid,
        Granted,
        Activated,
        Committed,
        Ended
    };
    
    class GameplayAbility : enable_shared_from_this<GameplayAbility>
    {
    public:
        AbilityType Type;
        GameplayAbilityState State;
        /** If true hides the ability from the player */
        bool bIsHidden = false;
        /** Cooldown in S, -1 = no CD*/
        float Cooldown = -1;

        std::shared_ptr<GameplayAbilityComponent> AssignedToComponent;

        /** Tags for the ability itself */
        GameplayTagContainer AbilityTags;
        /** Tags that the component to-be has to have */
        GameplayTagContainer ActivationRequiredTags;
        /** Tags on the assigned component that will trigger a deactivation */
        GameplayTagContainer DeActivationTriggerTags;

        GameplayAbility() : Type(AbilityType::DEFAULT), State(GameplayAbilityState::Invalid) {}
        virtual ~GameplayAbility() = default;
        virtual void Tick(float Delta);
        virtual bool ShouldTick();

        virtual bool ShouldActivate();
        virtual bool ShouldDeactivate();
        
        virtual void OnGranted();
        virtual void OnActivate();
        virtual void OnCommit();
        virtual void OnDeactivate();
        virtual void OnRemoved();
        
        ActionMap<GameplayAbilityComponentType, shared_ptr<GameplayAbilityComponent>> OnTargetHit;
        ActionMap<AbilityType, shared_ptr<GameplayAbility>> OnActivateAbility;
        ActionMap<AbilityType, shared_ptr<GameplayAbility>> OnEndAbility;

    protected:
        float CurrentCooldown = 0;
    };
}
