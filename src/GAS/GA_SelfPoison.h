#pragma once
#include "GAS/Abilities/GameplayAbility.h"

using namespace std;

class GA_SelfPoison : public GAS::GameplayAbility
{
public:
    bool ShouldActivate() override;
    void OnActivate() override;
};
