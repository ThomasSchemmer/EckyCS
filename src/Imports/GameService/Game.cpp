#include "Game.h"
#include <stdexcept>
#include <string>

#include "GameServiceDelegate.h"

namespace GameImports {

	Game* Game::Instance = nullptr;

	template<class T,  std::enable_if<std::is_base_of_v<GameService, T>>>
	T* Game::GetService(GameServiceType Type) {
		if (Game::Instance == nullptr)
			return nullptr;

		if (Game::Instance->ServicesInternal.contains(Type) &&
			!TryGetReplacementService<T>(Type))
			return nullptr;

		return Game::Instance->ServicesInternal[Type];
	}

	Game::Game()
	{
		Instance = this;
	}

	Game::~Game()
	{
		for (auto& Service: Services)
		{
			Service->StopService();
			delete Service;
		}
		Services.clear();
		
		for (auto& Delegate: Delegates)
		{
			delete Delegate;
		}
		Services.clear();
	}

	void Game::Init()
	{
		State = GameState::InGame;
		for (auto Service: Services)
		{
			ServicesInternal.emplace(Service->Type, Service);
			Service->StartService();
		}
	}

	GameService* Game::GetService(GameServiceType Type) {
		if (Game::Instance == nullptr)
			return nullptr;

		if (Game::Instance->ServicesInternal.contains(Type))
			return nullptr;

		return Game::Instance->ServicesInternal[Type];
	}

	template<class T, std::enable_if<std::is_base_of_v<GameService, T>>>
	bool Game::TryGetReplacementService(GameServiceType& FoundType)
	{
		FoundType = GameServiceType::INVALID;
		if (Game::Instance == nullptr)
			return false;

		for (const auto& pair : Game::Instance->ServicesInternal) {
			T* Ptr = dynamic_cast<T*>(pair.second);
			if (Ptr == nullptr)
				continue;

			FoundType = pair.first;
			return true;
		}
		return false;
	}

	void Game::DestroyServiceDelegate(GameServiceDelegate* Delegate) {
		auto it = ranges::find(Instance->Delegates, Delegate);
		if (it != Instance->Delegates.end()) {
			Instance->Delegates.erase(it);
		}

		auto Set = Delegate->GetRequiredServices();
		for (auto SetIt = Set.begin(); SetIt < Set.end(); ++SetIt)
		{
			// todo: make
			
		}

		delete Delegate;
	}


	void Game::RegisterCallback(const GameService* A, const GameService* B)
	{
		vector<GameServiceType> Chain;
		if (CheckForAnyLoopBetween(A, B, Chain))
		{
			const char* AN = typeid(*A).name();
			const char* BN = typeid(*B).name();
			std::string Message = "Infinite loop detected between: " + string(AN) + " and " + string(BN);
			throw std::runtime_error(Message);
		}

		// A->Type is implicitly always existing
		CallbackMap[A->Type].emplace(B->Type);
	}

	bool Game::CheckForAnyLoopBetween(const GameService* A, const GameService* B, vector<GameServiceType>& Chain)
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

	template<class T, std::enable_if<std::is_base_of_v<GameService, T>>>
	void Game::RunAfterServiceInit(Action<T> Callback){
		T Service = GetService<T>();
		if (Service == nullptr)
			return;

		// ignore the error for "not enough template args"!
		TemplatedDelegate<T> Delegate(Service, Callback, GameServiceDelegateType::OnInit);
		if (Delegate.HasRun())
			return;

		Instance->Delegates.emplace_back(Delegate);
		Instance->RegisterCallback(Callback.Target, Service);
	}
}
