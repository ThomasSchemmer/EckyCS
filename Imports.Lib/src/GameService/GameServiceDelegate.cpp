#include "GameServiceDelegate.h"

#include <algorithm>

#include "Game.h"

using namespace std;

namespace GameImports {
	
	int GameServiceDelegate::CURRENT_DELEGATE_ID = 0;
	
	vector<GameServiceType> GameServiceDelegate::GetRequiredServices() {
		vector<GameServiceType> Types;
		for (auto it = RequiredServices.begin(); it != RequiredServices.end(); ++it) {
			Types.emplace_back(it->first);
		}
		return Types;
	}

	bool GameServiceDelegate::HasRun() const
	{
		return bHasRun;
	}

	void GameServiceDelegate::MarkAsReady(GameServiceType ServiceType) {
		if (!RequiredServices.contains(ServiceType))
			return;

		RequiredServices[ServiceType] = true;
		RunIfReady();
	}

	void GameServiceDelegate::RunIfReady() {
		if (!AllServicesReady())
			return;

		ExecuteAction();
	}

	void GameServiceDelegate::ResetDelegates() {
		auto ReqSer = GetRequiredServices();
		//for (auto it = ReqSer.begin(); it < ReqSer.end(); ++it)
		//{
		//	switch (DelegateType) {
		//	case GameServiceDelegateType::OnInit:
		//		Game::GetService(*it)->OnInit.Remove(SourceType); break;
		//	case GameServiceDelegateType::OnStart:
		//		Game::GetService(*it)->OnStartup.Remove(SourceType); break;
		//	}
		//}
		
		Game::RemoveServiceDelegate(DelegateID);
	}

	

	bool GameServiceDelegate::AllServicesReady() const
	{
		for (const auto& Tuple : RequiredServices)
		{
			if (!Tuple.second)
				return false;
		}
		return true;
	}


}