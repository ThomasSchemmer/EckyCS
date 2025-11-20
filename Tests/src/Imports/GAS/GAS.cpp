
#include "../../../../ImportsLib/src/GAS/GameplayAbilityComponent.h"
#include "../../../../ImportsLib/src/GAS/GameplayAbilitySystem.h"
#include "../../../../ImportsLib/src/GAS/Attributes/AttributeSet.h"
#include "../../../../ImportsLib/src/GAS/Attributes/Attribute.h"
#include "../../../../ImportsLib/src/GAS/Attributes/AttributeType.h"
#include "../../../../ImportsLib/src/GameService/Game.h"
#include "gtest/gtest.h"

using namespace GAS;

TEST(Init, GAS)
{
    
    Game::Instance = make_unique<Game>([]() -> float{ return 0;});

    auto Gas = make_shared<GameplayAbilitySystem>();
    Game::Instance->Services.push_back(Gas);

    Game::Instance->Init();

    auto Comp = make_shared<GameplayAbilityComponent>();
    Gas->Register(Comp, GameplayAbilityComponentType::Player);

    auto AttrSet = make_shared<AttributeSet>();
    auto Health = make_shared<Attribute>();
    AttrSet->Add(AttributeType::Health, Health);
    Comp->Attributes = AttrSet;
    Comp->Init();
    // Expect Patchnotes
}