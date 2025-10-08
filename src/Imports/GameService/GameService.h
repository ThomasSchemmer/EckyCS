#pragma once
#include "../Util/ActionList.h"
#include <vector>

#ifndef INCLUDE_GUARD_GAME_SERVICE
#define INCLUDE_GUARD_GAME_SERVICE

namespace GameImports {
	enum class GameServiceType {
		INVALID,
		PlayerService,
	};

	/**
	 * Abstract class to allow the GameService to start / stop the attached script
	 */
	class GameService {
	public:
		virtual ~GameService() = default;
		GameService();
		void StartService();
		void StopService();
		void ResetService();

		ActionList<GameServiceType> OnStartup;
		ActionList<GameServiceType> OnShutdown;
		/** Needs to be manually called whenever the service is fully initialized - not all services will call it*/
		ActionList<GameServiceType> OnInit;
		GameServiceType Type = GameServiceType::INVALID;

	protected:
		virtual void StartServiceInternal() = 0;
		virtual void StopServiceInternal() = 0;
		virtual void ResetServiceInternal() = 0;

		bool bIsRunning = false;
		bool bIsInit = false;

	private:
		void BaseInit(GameServiceType ServiceType);
	};

}
#endif