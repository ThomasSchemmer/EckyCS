#pragma once
#include <memory>
#include <vector>

#include "ComponentGroupIdentifier.h"
#include "../../GameService/Game.h"
#include "../Util/EckyCSHeader.h"

namespace EckyCS
{
    using namespace std;
    using namespace GameImports;
    class ComponentGroupIdentifier;

    template <typename... Types>
    class ComponentGroupView
    {
        vector<shared_ptr<ComponentGroupIdentifier>> Groups;

    public:
        void Add(const shared_ptr<ComponentGroupIdentifier>& Group)
        {
            Groups.push_back(Group);
        }

        //void ForEach(EntityAction<Types...>& Action)
        //{s
        //    // TODO: build actual ECS to create systems etc
        //    Game::GetService<EckyCS>(GameServiceType::ECS)
        //    for (auto& Group : Groups)
        //    {
        //        Group->ForEach(Action);
        //    }
        //}
    };
}
