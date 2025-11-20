#pragma once
#include <iostream>

#include "Entity.h"
#include "../Util/EckyCSHeader.h"
#include "../Components/ComponentGroupIdentifier.h"
#include "../ECS.h"
#include "../../GameService/Game.h"

namespace EckyCS
{
    class EntityID;
    using namespace GameImports;

    class EntityGenerator
    {
    public:
        
        template <typename EntityType, typename... SuppliedComponents>
        requires HasRequiredComponents<EntityType>
        /**
         * Tries to create a new Entity and provides the resulting ID
         * Can take any selection of instantiated required components from the EntityType,
         * But: can only copy them, not ref/move - if you want to ref use @SparseSet.SetData after
         * registering/creating the Entity!
         */
        static bool TryCreate(ComponentGroupIdentifier& OutGroupID, EntityID& OutID, SuppliedComponents&... Components)
        {
            if (!Ecs && !((Ecs = Game::GetService<ECS>(GameServiceType::EntityComponentSystem))))
                return false;
            
            OutID = GetNextID();

            auto ProvidedTuple = make_tuple(Components...);
            using RequiredTuple = typename EntityType::RequiredComponents;
            auto AllComponents = CombineComponents<RequiredTuple>(ProvidedTuple);

            std::apply([&OutGroupID, &OutID]<typename... Args>(Args... Components){
                OutGroupID.AddFlags<std::remove_cvref_t<Args>...>();
                EntityGenerator::Ecs->RegisterEntity(OutGroupID, OutID, Components...);
            }, AllComponents);
            
            return true;
        }

        template <typename EntityType, typename... SuppliedComponents>
        requires HasRequiredComponents<EntityType>
        /**
         * Creates many entities from given Components memory
         * Warning: REQUIRES all components for the given type to be present
         * and in the correct order!
         */
        static bool TryCreateMany(size_t Count, SuppliedComponents*... Components)
        {
            if (!Ecs && !((Ecs = Game::GetService<ECS>(GameServiceType::EntityComponentSystem))))
                return false;
            
            auto ProvidedTuple = make_tuple(Components...);

            std::apply([&]<typename... Args>(Args*... InComponents){
                ComponentGroupIdentifier GroupID;
                GroupID.AddFlags<std::remove_cvref_t<Args>...>();
                EntityGenerator::Ecs->RegisterEntities(GroupID, GetNextIDs(Count), InComponents...);
            }, ProvidedTuple);
            
            return true;
        }
        
        inline static shared_ptr<ECS> Ecs = nullptr;

        static EntityID GetNextID()
        {
            return EntityID(CurrentID++); 
        }

        static vector<EntityID> GetNextIDs(size_t Count)
        {
            vector<EntityID> OutIDs;
            OutIDs.reserve(Count);
            for (size_t i = 0; i < Count; i++)
            {
                OutIDs.push_back(GetNextID());
            }
            return OutIDs;
        }

    private:
        template <typename Required, typename Given, size_t... I>
        /** For each Required type, tries to match from the Given or adds default*/
        static auto CombineComponents_Impl(Given& Values, index_sequence<I...>)
        {
            return make_tuple(
                GetOrDefault<tuple_element_t<I, Required>>(Values)...
            );
        }

        template <typename Required, typename Given>
        /** For each Required type, tries to match from the Given or adds default*/
        static auto CombineComponents(Given& Values)
        {
            constexpr size_t N = tuple_size_v<Required>;
            return CombineComponents_Impl<Required>(Values, make_index_sequence<N>{});
        }
       

        template <typename Target, typename Given, size_t... I>
        /** Iterates over Given values until a matching Target has been found, or creates an empty struct */
        static Target GetOrDefault_Impl(Given& Values, index_sequence<I...>)
        {
            auto Finder = [&](auto& Elem) -> Target*
            {
                // check what element type (ptr, self, ref,..) we are using
                // and cast accordingly
                using Current = remove_cvref_t<decltype(Elem)>;
                if constexpr (is_same_v<Current, Target>)
                    return &Elem;
                if constexpr (is_same_v<Current, Target*>)
                    return Elem;
                if constexpr (is_same_v<Current, shared_ptr<Target>>)
                    return Elem.get();
                return nullptr;
            };

            Target* Result = nullptr;
            
            // expand over I: 
            (
                // keep Result if you have found it already, otherwise actually go find it
                (Result = Result != nullptr ? Result :
                    Finder(get<I>(Values))
                ),
            ...);
            if (Result)
                return *Result; 

            return Target{};
        }

        template <typename Target, typename Given>
        /** Finds the Target inside of Given values, or returns an empty Target*/
        static Target GetOrDefault(Given& Values)
        {
            constexpr size_t N = tuple_size_v<Given>;
            return GetOrDefault_Impl<Target>(Values, make_index_sequence<N>{});
        }
        
        inline static int CurrentID = 1;
    };
}
