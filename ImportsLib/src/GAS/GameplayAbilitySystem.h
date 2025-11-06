#pragma once
#include <map>
#include <memory>

#include "../GameService/GameService.h"

namespace GAS
{
    using namespace std;
    using namespace GameImports;
    enum class GameplayAbilityComponentType : uint8_t;
    class GameplayAbilityComponent;
    class GameplayEffect;
    class GameplayAbility;
    
    /**
     * Core component of the GAS, handles communication between different GAC's
     * Only one should exist in the world
     * 
     * Ability: Fireball
     * -> create Entity Projectile
     * -> on hit effect: FireballExplode
     *
     * Effect: FireballExplode
     * -> OnHitModifier: take 5 fire dmg
     * -> DotModifier: take 1dps fire for 5 sec
     *
     * OnHitModifier: (0 period), value = 5 -> Attribute
     * DotModifier: (1 period), value = 1 -> Attribute
     */
    class GameplayAbilitySystem : public GameService
    {
    public:
        std::map<GameplayAbilityComponentType, vector<shared_ptr<GameplayAbilityComponent>>> Components;

        void Update() const;
        void Register(const shared_ptr<GameplayAbilityComponent>& Component, GameplayAbilityComponentType ComponentType);
        void DeRegister(const shared_ptr<GameplayAbilityComponent>& Component, GameplayAbilityComponentType ComponentType);

        static bool TryApplyEffectTo(const shared_ptr<GameplayAbilityComponent>& Target, const shared_ptr<GameplayEffect>& Effect);
        static bool TryGiveAbilityTo(const shared_ptr<GameplayAbilityComponent>& Target, const shared_ptr<GameplayAbility>& Ability);
        static bool TryActivateAbility(const shared_ptr<GameplayAbilityComponent>& Target, const shared_ptr<GameplayAbility>& Ability);

        ActionMap<GameplayAbilityComponentType, shared_ptr<GameplayAbilityComponent>> OnComponentRegistered;
        
    protected:
        void StartServiceInternal() override;
        void StopServiceInternal() override;
        void ResetServiceInternal() override;
    };
}
