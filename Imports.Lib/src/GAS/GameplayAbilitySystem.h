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
     * Core component of the GAS, handles communication between different GAB's
     * Only one should exist in the world
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
