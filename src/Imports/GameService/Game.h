#pragma once
#include <map>
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
	 * Centerpoint of the whole game, contains main()
	 * Holds info about state, mode and available services
	 */
	class Game {
	private:
		map<GameServiceType, GameService*> ServicesInternal;

	public:
		GameState State = GameState::InGame;
		GameMode Mode = GameMode::InGameMode;
		bool bIsPaused = false;
		int TargetPlayerCount = 1;

		vector<GameService*> Services;
		vector<GameServiceDelegate*> Delegates;
		map<GameServiceType, unordered_set<GameServiceType>> CallbackMap;
		
		static Game* Instance;

		Game();
		~Game();

		/** Starts the initialization of the whole game - should be run before any frames*/
		void Init();
		
		/** Returns the best fitting service according to type */
		template<class T, std::enable_if<std::is_base_of_v<GameService, T>>>
		static T* GetService(GameServiceType Type);

		/** Returns the service according to type */
		static GameService* GetService(GameServiceType Type);

		/** Returns the best fitting type of any registered services*/
		template<class T, std::enable_if<std::is_base_of_v<GameService, T>>>
		static bool TryGetReplacementService(GameServiceType& FoundType);

		/** Destroys and fully removes a delegate*/
		static void DestroyServiceDelegate(GameServiceDelegate* Delegate);

		/** Creates a delegate that will run once the specified service is initialized */
		template<class T, std::enable_if<std::is_base_of_v<GameService, T>>>
		static void RunAfterServiceInit(Action<T> Callback);

	private:
		void RegisterCallback(const GameService* A, const GameService* B);
		bool CheckForAnyLoopBetween(const GameService* A, const GameService* B, vector<GameServiceType>& Chain);
	};
}
