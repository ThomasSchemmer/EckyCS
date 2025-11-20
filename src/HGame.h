#pragma once
#include "TestService.h"
#include "EckyCS/ECS.h"
#include "EckyCS/Systems/ItemMovementSystem.h"
#include "EckyCS/Systems/ItemRenderSystem.h"
#include "GameService/Game.h"

class HGame : public Game
{
public:
	HGame(const std::function<float()>& TF) : Game(TF)
	{
		auto Ecs = std::make_shared<ECS>();
		Ecs->AddSystem(std::make_shared<ItemRenderSystem>());
		Ecs->AddSystem(std::make_shared<ItemMovementSystem>());
		
		Services.push_back(Ecs);
		Services.push_back(make_shared<TestService>());
	}

	void Update() override
	{
		Game::Update();
	}
};
