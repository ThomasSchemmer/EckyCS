#include "GameService.h"

namespace GameImports {

	GameService::GameService() {
		auto Temp = std::bind(&GameService::BaseInit, this, std::placeholders::_1);
		OnInit.Add(Temp);
	}

	void GameService::StartService() {
		bIsRunning = true;
		StartServiceInternal();
		OnStartup.ForEach(Type);
	}

	void GameService::StopService() {
		bIsRunning = false;
		StopServiceInternal();
		OnShutdown.ForEach(Type);
	}
	
	void GameService::ResetService()
	{
		ResetServiceInternal();
		bIsRunning = false;
	}

	void GameService::BaseInit(GameServiceType ServiceType)
	{
		bIsInit = true;
	}
}