#pragma once
#include "ComponentGroupView.h"


namespace EckyCS
{
    class ISparseSet;
    
    class IComponentGroupProvider
    {
    public:

        template <typename ... Types>
        requires AllComponents<Types...>
        ComponentGroupView Get()
        {
            ComponentGroupView GroupView;
            for (const auto& Tuple : GetViewSet())
            {
                if (!Tuple.first.HasAllFlags<Types...>())
                    continue;

                auto Copy = make_shared<ComponentGroupIdentifier>(Tuple.first);
                GroupView.Add(Copy);
            }
            return GroupView;
        }

        virtual ~IComponentGroupProvider() = default;
        
    protected:
        virtual map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> GetViewSet() = 0;
    };
}
