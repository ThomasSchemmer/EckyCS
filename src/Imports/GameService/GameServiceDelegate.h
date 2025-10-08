#pragma once
#include "GameService.h"
#include "Game.h"
#include <map>

namespace GameImports {
	using namespace std;

	enum class GameServiceDelegateType : uint8_t {
		OnStart,
		OnInit
	};

	class GameServiceDelegate {
	public:
		virtual ~GameServiceDelegate() = default;
		virtual void ExecuteAction() = 0;
		virtual void ResetAction() = 0;

		vector<GameServiceType> GetRequiredServices();
		bool HasRun();

	protected:
		map<GameServiceType, bool> RequiredServices;
		GameServiceDelegateType Type = GameServiceDelegateType::OnInit;
		bool bHasRun = false;
	
		void MarkAsReady(GameServiceType ServiceType);
		void RunIfReady();
		virtual void ResetDelegates();

	private:
		bool AllServicesReady();

	};

    template<class T, std::enable_if<std::is_same_v<GameServiceType, T>>>
    class TemplatedDelegate : public GameServiceDelegate
    {
    public:
        Action<T> A;

        void ExecuteAction() override
        {
            if (A == nullptr)
                return;

            vector<GameServiceType> StartedServices = GetRequiredServices();
            bHasRun = true;
            ResetDelegates();
            A(Game::GetService(StartedServices[0]));
            ResetAction();
        }

        void ResetAction() override
        {
            A = nullptr;
        }

        TemplatedDelegate(T RequiredService, Action<T> Callback, GameServiceDelegateType DelegateType = GameServiceDelegateType::OnStart)
        {
            bool bIsReady = DelegateType == GameServiceDelegateType::OnStart ? RequiredService.IsRunning : RequiredService.IsInit;
            RequiredServices.emplace(RequiredService, bIsReady);
            A = Callback;
            Type = DelegateType;
            switch (DelegateType)
            {
            case GameServiceDelegateType::OnStart:
                RequiredService.OnStartup += MarkAsReady;
                break;
            case GameServiceDelegateType::OnInit:
                RequiredService.OnInit += MarkAsReady;
                break;
            }
            RunIfReady();
        }
    };
}
