#include "ECS.h"

#include "SparseSet/SparseSet.h"
#include "Systems/System.h"
#include "../GameService/Game.h"

using namespace std;
namespace EckyCS
{
    void ECS::Update()
    {
        GameService::Update();
        ForEachSystem([](const shared_ptr<System>& S)
        {
           S->Tick(Game::DeltaTime);
        });
    }

    void ECS::FixedUpdate()
    {
        GameService::FixedUpdate();
        ForEachSystem([](const shared_ptr<System>& S)
        {
           S->FixedTick(Game::DeltaFixedTime);
        });
    }

    ECS::ECS()  
    {
        Type = GameServiceType::EntityComponentSystem;
    }

    ComponentGroupIdentifier ECS::GetOrCreateGroupIDFor(const EntityID& ID)
    {
        if (!EntityMapping.contains(ID))
        {
            EntityMapping[ID] = EmptyGroup;
            Sets[EmptyGroup]->Add(ID);
        }
        return EntityMapping[ID];
    }

    void ECS::StartServiceInternal()
    {
        GetOrCreateSet(EmptyGroup);
        // TODO: add location system
        auto Ptr = shared_from_this();
        ForEachSystem([&Ptr](const shared_ptr<System>& S)
        {
            S->StartSystem(Ptr);
        });
        OnInit.ForEach(Type);
    }

    void ECS::ForEachSystem(function<void(const shared_ptr<System>&)> Func) const
    {
        for (auto& List : Systems)
        {
            for (auto& System : List.second)
            {
                Func(System);
            }
        }
    }

    ECS::~ECS()
    {
        for (auto& Pair : Systems)
        {
            Pair.second.clear();
        }
        Systems.clear();
    }

    void ECS::StopServiceInternal()
    {
    }

    void ECS::ResetServiceInternal()
    {
    }

    map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> ECS::GetViewSet()
    {
        return this->Sets;
    }
}
