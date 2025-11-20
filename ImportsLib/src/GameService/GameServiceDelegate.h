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

	/**
	 * Container for any action delegation in regards to GameServices
	 * aka "do something after a service has started/initialized"
	 */
	class GameServiceDelegate : enable_shared_from_this<GameServiceDelegate>
	{
	public:
		virtual ~GameServiceDelegate() = default;
		virtual void ExecuteAction() = 0;
		virtual void ResetAction() = 0;

		vector<GameServiceType> GetRequiredServices();
		void MarkAsReady(GameServiceType ServiceType);
		bool HasRun() const;
		
		GameServiceType SourceType;
		
	protected:
		map<GameServiceType, bool> RequiredServices;
		GameServiceDelegateType DelegateType = GameServiceDelegateType::OnInit;
		
		bool bHasRun = false;
		int DelegateID = -1;
	
		void RunIfReady();
		virtual void ResetDelegates();

		static int CURRENT_DELEGATE_ID;

	private:
		
		bool AllServicesReady() const;

	};
	
	template<class T>
	requires std::is_base_of_v<GameService, T>
    class TemplatedDelegate : public GameServiceDelegate, enable_shared_from_this<TemplatedDelegate<T>>
    {
    public:
        Action<shared_ptr<T>> A;

        void ExecuteAction() override
        {
            if (A == nullptr)
                return;

            vector<GameServiceType> StartedServices = GetRequiredServices();
            bHasRun = true;

        	const auto Ptr = Game::GetService<T>(StartedServices[0]);
        	if (Ptr == nullptr)
        		return;
        	
            A(Ptr);
            ResetAction();
            ResetDelegates();
        }

        void ResetAction() override
        {
            A = nullptr;
        }

        TemplatedDelegate(GameServiceType NewSourceType, GameServiceType ServiceType, Action<shared_ptr<T>> Callback, GameServiceDelegateType NewDelegateType = GameServiceDelegateType::OnStart)
        {
        	// if we are calling from a non-service, the source will be invalid - the call will be executed anyways
        	// but no callback detection is possible, so just drop that part
        	const auto SourceService = Game::GetService(NewSourceType);
        	const auto TargetService = Game::GetService<T>(ServiceType);
        	if ((NewSourceType != GameServiceType::INVALID && SourceService == nullptr) || TargetService == nullptr)
				throw std::exception("ERROR::GAME:INVALID_DELEGATE_TYPE");
        	
			const bool bIsReady = TargetService->IsReadyFor(NewDelegateType);
            RequiredServices.emplace(std::make_pair(TargetService->Type, bIsReady));
            A = Callback;
            this->DelegateType = NewDelegateType;
        	this->SourceType = SourceService ? SourceService->Type : GameServiceType::INVALID;
        	
        	DelegateID = ++CURRENT_DELEGATE_ID;
        	const int Temp = DelegateID;
        	auto Lambda = [Temp](GameServiceType ServiceType)
        	{
        		Game::MarkAsReadyFor(Temp, ServiceType);
        	};
            switch (NewDelegateType)
            {
            case GameServiceDelegateType::OnStart:
            	TargetService->OnStartup.Add(NewSourceType, Lambda);
                break;
            case GameServiceDelegateType::OnInit:
            	TargetService->OnInit.Add(NewSourceType, Lambda);
                break;
            }
        }

    	
    	/**
    	 * Creates a delegate that will run once the specified service is initialized
    	 * T: GameService that is required
    	 */
		static void RunAfterServiceInit(Action<shared_ptr<T>> Callback, GameServiceType NewSourceType, GameServiceType ServiceType){
        	auto Delegate = make_shared<TemplatedDelegate>(NewSourceType, ServiceType, Callback, GameServiceDelegateType::OnInit);
        	Game::Instance->Delegates.emplace(std::make_pair(Delegate->DelegateID, Delegate));

        	Delegate->RunIfReady();
        	if (Delegate->HasRun()) // also removes delegate from map
        		return;
        	
        	const auto SourceService = Game::GetService(NewSourceType);
        	const auto TargetService = Game::GetService<T>(ServiceType);
        	if (SourceService == nullptr || TargetService == nullptr)
        		return;

        	Game::RegisterCallback(SourceService, TargetService);
        }

    };
}
