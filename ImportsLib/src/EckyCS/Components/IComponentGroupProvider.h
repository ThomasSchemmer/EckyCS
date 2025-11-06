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
        ComponentGroupView<Types...> Get()
        {
            ComponentGroupView<Types...> GroupView;
            for (const auto& Tuple : GetViewSet())
            {
                if (!Tuple.first.HasAllFlags<Types...>())
                    continue;

                GroupView.Add(Tuple.second);
            }
            return GroupView;
        }

        virtual ~IComponentGroupProvider() = default;
        
    protected:
        virtual map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> GetViewSet() = 0;
    };
}
