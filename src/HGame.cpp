#include "HGame.h"

#include "EckyCS/Systems/ItemRenderSystem.h"
#include "GAS/AC_Player.h"
#include "GAS/GameplayAbilitySystem.h"
#include "GAS/GA_SelfPoison.h"

HGame::HGame(const std::function<float()>& TF) : Game(TF)
{
    auto Ecs = make_shared<ECS>();
    Ecs->AddSystem(make_shared<ItemRenderSystem>());
    Ecs->AddSystem(make_shared<ItemMovementSystem>());

    Services.push_back(Ecs);
    Services.push_back(make_shared<RenderTestService>());

    auto GAS = make_shared<GameplayAbilitySystem>();
    Services.push_back(GAS);
}

void HGame::Init(GLFWwindow* Window)
{
    Game::Init(Window);

    auto MainAbilityComponent = make_shared<AC_Player>();
    MainAbilityComponent->Init();
    auto MainPlayerController = make_shared<PlayerController>(MainAbilityComponent);

    auto GA_SelfPoisonInstance = make_shared<GA_SelfPoison>();
    auto GAS = GetService<GameplayAbilitySystem>(GameServiceType::GameplayAbilitySystem);
    GAS->TryGiveAbilityTo(MainAbilityComponent, GA_SelfPoisonInstance);
}
