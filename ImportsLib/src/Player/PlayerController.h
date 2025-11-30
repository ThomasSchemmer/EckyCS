#pragma once
#include "../GAS/GameplayAbilityComponent.h"

using namespace GAS;

namespace Player
{
    class PlayerController
    {
    public:
        explicit PlayerController(const shared_ptr<GameplayAbilityComponent>& InAbilityComponent)
            : AbilityComponent{InAbilityComponent}
        {
        }

        shared_ptr<GameplayAbilityComponent> AbilityComponent;
    };
}
