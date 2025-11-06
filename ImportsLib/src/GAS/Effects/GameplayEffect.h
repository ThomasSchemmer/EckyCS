#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "../Abilities/GameplayAbility.h"
#include "../GameplayAbilityComponent.h"
#include "../Abilities/GameplayAbilityInstanceable.h"

namespace GAS
{
    using namespace std;
    class GameplayEffectModifier;
    enum class GameplayEffectModifierType : uint8_t;
    
    enum class GameplayEffectDuration : uint8_t
    {
        Instant,
        Duration,
        Infinite
    };

    /**
     * Baseclass that contains @GameplayEffectModifier to modify
     * @Attribute's through @Ability's. 
     */
    class GameplayEffect : public GameplayAbilityInstanceable
    {
    public:
        GameplayEffectDuration DurationType;
        std::vector<shared_ptr<GameplayEffectModifier>> Modifiers;
        float DurationLength = 0;
        shared_ptr<GameplayAbility> GrantedAbility;

        void SetTarget(shared_ptr<GameplayAbilityComponent> NewTarget);
        void Execute() const;
        void Deactivate();
        void Tick(float Delta) const;
        bool IsExpired(float Delta);

        std::vector<shared_ptr<GameplayEffectModifier>> GetModifiersByOperation(GameplayEffectModifierType OperationType) const;
        std::string GetEffectDescription() const;

        /** Tags that are granted to the target on assignment*/
        GameplayTagContainer GrantedTags;
        /** Tags that are required on the target to be applied, but only at the moment of application */
        GameplayTagContainer ApplicationRequiredTags;
        /** Tags that have to continually be present on the target while the effect is applied */
        GameplayTagContainer OngoingRequirementTags;
        /** Tags that will be removed from the target on application */
        GameplayTagContainer RemoveTags;

    protected:
        bool IsSelfOnActor(const shared_ptr<GameplayAbilityComponent>& Actor) override;
        
    private:
        shared_ptr<GameplayAbilityComponent> Target;
        float Runtime = 0;
        bool bIsDeactivated = false;
    };
}
