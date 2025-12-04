#pragma once
#include "Util/RenderTestService.h"
#include "EckyCS/ECS.h"
#include "EckyCS/Systems/ItemMovementSystem.h"
#include "GameService/Game.h"
#include "Player/PlayerController.h"

using namespace GAS;
using namespace Player;

class HGame : public Game
{
public:
    HGame(const std::function<float()>& TF);

    void Init(shared_ptr<GLFWwindow>& Window) override;

    void Update() override
    {
        Game::Update();
    }
};
