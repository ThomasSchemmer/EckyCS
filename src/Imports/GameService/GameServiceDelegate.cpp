#include "GameServiceDelegate.h"
#include "Game.h"

using namespace std;

namespace GameImports {
	vector<GameServiceType> GameServiceDelegate::GetRequiredServices() {
		vector<GameServiceType> Types;
		for (auto it = RequiredServices.begin(); it != RequiredServices.end(); ++it) {
			Types.emplace_back(it->first);
		}
		return Types;
	}

	bool GameServiceDelegate::HasRun() { return false; }

	void GameServiceDelegate::MarkAsReady(GameServiceType ServiceType) {
		if (RequiredServices.contains(ServiceType))
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
		for (auto it = ReqSer.begin(); it < ReqSer.end(); ++it) {

			auto Temp = std::bind(&GameServiceDelegate::MarkAsReady, this, std::placeholders::_1);
			switch (Type) {
			case GameServiceDelegateType::OnInit:
				Game::GetService(*it)->OnInit.Remove(Temp);
			case GameServiceDelegateType::OnStart:
				Game::GetService(*it)->OnStartup.Remove(Temp);
			}
		}
		Game::DestroyServiceDelegate(this);
	}

	bool GameServiceDelegate::AllServicesReady() { return false; }


}