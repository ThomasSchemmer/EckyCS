#pragma once
#include <memory>
#include <vector>

#include "ComponentGroupIdentifier.h"
#include "GameService/Game.h"


namespace EckyCS
{
    using namespace std;
    using namespace GameImports;

    class ComponentGroupView
    {
    public:
        vector<shared_ptr<ComponentGroupIdentifier>> Groups;

        void Add(const shared_ptr<ComponentGroupIdentifier>& Group)
        {
            Groups.push_back(Group);
        }

        
    };
}
