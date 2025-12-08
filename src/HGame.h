#pragma once
#include "EckyCS/ECS.h"
#include "EckyCS/Systems/ItemMovementSystem.h"
#include "GameService/Game.h"
#include "Player/PlayerController.h"
#include "Util/RenderTestService.h"

using namespace GAS;
using namespace Player;

class HGame : public Game
{
public:
    HGame(const std::function<float()>& TF);

    void Init(GLFWwindow* Window) override;

    void Update() override
    {
        Game::Update();
    }
};
