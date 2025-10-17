#pragma once
#include "PlayerService.h"
#include "GameService/GameServiceDelegate.h"

class PlayerService;

namespace GAS
{
    class GameplayAbilityComponent;
    class GameplayAbilitySystem;
}

using namespace GameImports;
class TestService : public GameService, public enable_shared_from_this<TestService>
{
public:
    TestService() 
    {
        Type = GameServiceType::Test;
    }
    
    void StartServiceInternal () override
    {
        OnInit.ForEach(Type);
        TemplatedDelegate<PlayerService>::RunAfterServiceInit([this](const shared_ptr<PlayerService>& Players)
              {
                  Players->PrintTest();
              },
              GameServiceType::Test,
              GameServiceType::Player
        );
    }
    
    void StopServiceInternal() override {}
    void ResetServiceInternal() override {}
};
