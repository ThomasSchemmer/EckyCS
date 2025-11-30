#pragma once
#include "GAS/GameplayAbilityComponent.h"
#include "GAS/Attributes/AttributeSet.h"

using namespace GAS;

class AC_Player: public GameplayAbilityComponent
{
public:
    AC_Player()
    {
        Type = GameplayAbilityComponentType::DEFAULT;
        Attributes = make_shared<AttributeSet>();
    }
};
