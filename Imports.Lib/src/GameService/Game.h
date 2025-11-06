#pragma once
#include <map>
#include <memory>
#include <unordered_set>

#include "GameService.h"

namespace GameImports {
	class GameServiceDelegate;
	using namespace std;
	
	enum class GameState : uint8_t{
		InGame,
		GameMenu,
		MainMenu
	};

	enum class GameMode : uint8_t{
		InGameMode
	};

	/**
	 * Centerpoint of the whole game
	 * Holds info about state, mode and available services
	 */
	class Game 
	{
	private:
		map<GameServiceType, shared_ptr<GameService>> ServicesInternal;
		float LastFixedTick = 0;
		float LastTick = 0;
		
		// the whole project shouldn't include glfw just for the time 
		function<float()> TimeFunction;
		
	public:
		GameState State = GameState::InGame;
		GameMode Mode = GameMode::InGameMode;
		bool bIsPaused = false;
		int TargetPlayerCount = 1;

		vector<shared_ptr<GameService>> Services;
		map<int, shared_ptr<GameServiceDelegate>> Delegates;
		map<GameServiceType, unordered_set<GameServiceType>> CallbackMap;
		
		static unique_ptr<Game> Instance;
		static float DeltaTime, DeltaFixedTime;

		Game(const function<float()>& TF) : TimeFunction(TF) {}
		~Game();

		/** Starts the initialization of the whole game - should be run before any frames*/
		void Init();

		void Update();
		void FixedUpdate() const;
		
		/** Returns the best fitting service according to type */
		template<class T>
		requires std::is_base_of_v<GameService, T>
		static shared_ptr<T> GetService(GameServiceType Type) {
			if (Instance == nullptr)
				return nullptr;

			if (Instance->ServicesInternal.contains(Type) &&
				!TryGetReplacementService<T>(Type))
				return nullptr;

			return dynamic_pointer_cast<T>(Instance->ServicesInternal[Type]);
		}

		/** Returns the service according to type */
		static shared_ptr<GameService> GetService(GameServiceType Type);

		/** Returns the best fitting type of any registered services*/
		template<class T>
		requires std::is_base_of_v<GameService, T>
		static bool TryGetReplacementService(GameServiceType& FoundType)
		{
			FoundType = GameServiceType::INVALID;
			if (Instance == nullptr)
				return false;

			for (const auto& pair : Game::Instance->ServicesInternal) {
				auto Ptr = dynamic_pointer_cast<T>(pair.second);
				if (Ptr == nullptr)
					continue;

				FoundType = pair.first;
				return true;
			}
			return false;
		}

		/**
		 * Fully deregisters a delegate but doesn't destroy it
		 * Cannot use shared_ptr as its bing called inside a function delegate, so cannot
		 * use "shared_from_this"
		 */
		static void RemoveServiceDelegate(int DelegateID);
		static void MarkAsReadyFor(int DelegateID, GameServiceType ServiceType);
		static void RegisterCallback(const shared_ptr<GameService>& A, const shared_ptr<GameService>& B);
	private:
		void RemoveCallback(GameServiceType A, GameServiceType B);
		bool CheckForAnyLoopBetween(const shared_ptr<GameService>& A, const shared_ptr<GameService>& B, vector<GameServiceType>& Chain);

		static constexpr double FIXED_TICK_INTERVAL = 1 / 24.0;
	};
}
