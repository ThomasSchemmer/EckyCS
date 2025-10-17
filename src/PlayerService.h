#pragma once
#include "GameService/GameService.h"

using namespace GameImports;

class PlayerService : public GameService {
public:

	PlayerService() 
	{
		Type = GameServiceType::Player;
	}
	
	void StartServiceInternal () override
	{
		OnInit.ForEach(Type);
	}

	void PrintTest()
	{
		std::cout << "Hi there" << "\n";
	}
	void StopServiceInternal() override {}
	void ResetServiceInternal() override {}
};