#pragma once
#include "../Util/ActionMap.h"
#include <vector>

namespace GameImports {
	enum class GameServiceDelegateType : uint8_t;

	enum class GameServiceType {
		INVALID,
		Player,
		GameplayAbilitySystem,
		EntityComponentSystem,
		Test,
	};

	/**
	 * Class that provides unified access to th different services the game interacts with
	 * Eg: InventorySystem, PlayerInstancing, ObjectLoading..
	 */
	class GameService {
	public:
		virtual ~GameService() = default;
		GameService();
		virtual void Update(float Delta) {}
		virtual void FixedUpdate(float Delta) {}
		virtual void StartService();
		virtual void StopService();
		virtual void ResetService();
		virtual bool IsReadyFor(GameServiceDelegateType TargetDelegateType) const;

		ActionMap<GameServiceType, GameServiceType> OnStartup;
		ActionMap<GameServiceType, GameServiceType> OnShutdown;
		/** Needs to be manually called whenever the service is fully initialized - not all services will call it*/
		ActionMap<GameServiceType, GameServiceType> OnInit;
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