#include "Game.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include "GameServiceDelegate.h"
#include "../Renderer/Renderer.h"
#include "../Player/PlayerController.h"

using namespace TTerrain;
using namespace Player;

namespace GameImports {

	unique_ptr<Game> Game::Instance = nullptr;
	float Game::DeltaTime = 0;
	float Game::DeltaFixedTime = 0;

	Game::Game(const function<float()>& TF): TimeFunction(TF)
	{
		RendererPtr = make_shared<Renderer>();
		TerrainPtr = make_shared<TerrainManager>();
	}

	Game::~Game()
	{
		for (auto& Service: Services)
		{
			Service->StopService();
			Service.reset();
		}
		Services.clear();
		
		for (auto& Pair: Delegates)
		{
			Pair.second.reset();
		}
		Services.clear();
	}

	void Game::Init(GLFWwindow* Window)
	{
		State = GameState::InGame;
		WindowPtr = Window;
		
		for (auto& Service: Services)
		{
			ServicesInternal.emplace(Service->Type, Service);
		}
		
		for (auto& Service: Services)
		{
			Service->StartService();
		}
		
		RendererPtr->Init(Window);
		TerrainPtr->Init();
	}

	void Game::Update()
	{
		float Now = TimeFunction();
		DeltaTime = Now - LastTick;
		for (auto& Service: Services)
		{
			Service->Update();
		}
		if (Now - LastFixedTick > FIXED_TICK_INTERVAL)
		{
			DeltaFixedTime = Now - LastFixedTick;
			FixedUpdate();
			LastFixedTick = Now;
		}
		LastTick = Now;
	}

	void Game::FixedUpdate() const
	{
		for (auto& Service: Services)
		{
			Service->FixedUpdate();
		}
	}

	shared_ptr<GameService> Game::GetService(GameServiceType Type) {
		if (Instance == nullptr)
			return nullptr;

		if (!Instance->ServicesInternal.contains(Type))
			return nullptr;

		return Instance->ServicesInternal[Type];
	}

	void Game::RemoveServiceDelegate(int DelegateID) {
		auto& Delegate = Instance->Delegates[DelegateID];
		auto& Delegates = Instance->Delegates;

		auto Set = Delegate->GetRequiredServices();
		for (auto& Other : Set)
		{
			Instance->RemoveCallback(Delegate->SourceType, Other);
		}

		Instance->Delegates.erase(DelegateID);
	}

	void Game::MarkAsReadyFor(int DelegateID, GameServiceType ServiceType)
	{
		auto& Delegate = Instance->Delegates[DelegateID];
		Delegate->MarkAsReady(ServiceType);
	}


	void Game::RegisterCallback(const shared_ptr<GameService>& A, const shared_ptr<GameService>& B)
	{
		if (Instance == nullptr)
			return;
		
		vector<GameServiceType> Chain;
		if (Instance->CheckForAnyLoopBetween(A, B, Chain))
		{
			const char* AN = typeid(A.get()).name();
			const char* BN = typeid(B.get()).name();
			std::string Message = "Infinite loop detected between: " + string(AN) + " and " + string(BN);
			throw std::runtime_error(Message);
		}

		// A->Type is implicitly always existing
		Instance->CallbackMap[A->Type].emplace(B->Type);
	}

	void Game::RemoveCallback(GameServiceType A, GameServiceType B)
	{
		if (A == GameServiceType::INVALID || !CallbackMap.contains(A) || !CallbackMap[A].contains(B))
			return;

		CallbackMap[A].erase(B);
	}

	bool Game::CheckForAnyLoopBetween(const shared_ptr<GameService>& A, const shared_ptr<GameService>& B, vector<GameServiceType>& Chain)
	{
		if (A == nullptr || B == nullptr)
			return false;

		if (A == B)
		{
			Chain.emplace_back(A->Type);
			return true;
		}

		if (CallbackMap.contains(B->Type))
			return false;

		auto& BMap = CallbackMap[B->Type];
		for (auto it = BMap.begin(); it != BMap.end(); ++it)
		{
			vector<GameServiceType> PrevChain;
			if (!CheckForAnyLoopBetween(A, Game::GetService(*it), PrevChain))
				continue;

			Chain.emplace_back(B->Type);
			Chain.insert(Chain.end(), PrevChain.begin(), PrevChain.end());
			return true;
		}
		return false;
	}

	
}
