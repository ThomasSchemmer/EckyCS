#pragma once

#include "./Imports/GameService/GameService.h"
using namespace GameImports;

class PlayerService : public GameService {
public:
	//using GameService::GameService;
	PlayerService() 
	{
		Type = GameServiceType::PlayerService;
	}
	
	void StartServiceInternal () override
	{
		OnInit.ForEach(Type);
	}
	void StopServiceInternal() override {}
	void ResetServiceInternal() override {}
};