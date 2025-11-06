#include "GameService.h"

#include "GameServiceDelegate.h"

namespace GameImports {

	GameService::GameService() {
		auto Temp = std::bind(&GameService::BaseInit, this, std::placeholders::_1);
		OnInit.Add(Type, Temp);
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

	bool GameService::IsReadyFor(GameServiceDelegateType TargetDelegateType) const
	{
		return
			TargetDelegateType == GameServiceDelegateType::OnStart ? bIsRunning : bIsInit;
	}

	void GameService::BaseInit(GameServiceType ServiceType)
	{
		bIsInit = true;
		Type = ServiceType;
	}
}
