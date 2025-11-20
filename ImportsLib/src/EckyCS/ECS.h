#pragma once
#include <cassert>
#include <memory>

#include "../GameService/GameService.h"
#include "Components/IComponentGroupProvider.h"
#include "SparseSet/ISparseSet.h"
#include "SparseSet/SparseSet.h"
#include "Util/TypeInfo.h"

namespace EckyCS
{
    class System;
    class ComponentGroupIdentifier;
    enum class SystemType : uint8_t;
    using namespace GameImports;
    using namespace std;

    
    
    /**
     * Central class for the ECS - should be the only class that you directly interact with
     * Is a gameservice for added convenience 
     */
    class ECS : public GameService, public IComponentGroupProvider, public enable_shared_from_this<ECS>
    {
        map<TypeInfo, vector<shared_ptr<System>>> Systems;
        map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> Sets;
        map<EntityID, ComponentGroupIdentifier> EntityMapping;
        
        ComponentGroupIdentifier EmptyGroup;
        
    public:
        void Update() override;
        void FixedUpdate() override;
        ECS();
        ~ECS() override;

        
        template <typename... Types>
        shared_ptr<ISparseSet> GetOrCreateSet(const ComponentGroupIdentifier& GroupID)
        {
            if (!Sets.contains(GroupID))
            {
                Sets.emplace(make_pair(GroupID, make_shared<SparseSet<Types...>>()));
            }
            return GetSet(GroupID);
        }
        
        shared_ptr<ISparseSet> GetSet(const ComponentGroupIdentifier& GroupID)
        {
            assert(Sets.contains(GroupID));
            return Sets[GroupID];
        }
        
        ComponentGroupIdentifier GetOrCreateGroupIDFor(const EntityID& ID);

        template <typename S>
        requires std::is_base_of_v<System, S>
        void AddSystem(const shared_ptr<S>& System)
        {
            auto& Type = typeid(System.get());
            Systems[Type].emplace_back(System);
            if (!bIsInit)
                return;

            auto Ptr = shared_from_this();
            System->StartSystem(Ptr);
        }

        template<typename T>
        bool TryGetSystem(shared_ptr<System>& OutSystem) const
        {
            vector<shared_ptr<System>> List;
            if (!TryGetSystems<T>(List))
                return false;

            OutSystem = List[0];
            return true;
        }

        template<typename T>
        bool TryGetSystems(vector<shared_ptr<System>>& OutList) const
        {
            auto& InType = typeid(T);
            if (Systems.contains(InType))
                return false;

            for (const auto& Tuple : Systems)
            {
                if (Tuple.second.empty())
                    continue;

                // sadly we don't have automatic type_info inheritance info,
                // so we always have to manually check. Will be performance heavy!
                auto Ptr = reinterpret_pointer_cast<T>(Tuple.second[0]);
                if (!Ptr)
                    continue;

                OutList.insert(OutList.end(), Tuple.second.begin(), Tuple.second.end());
            }

            return !OutList.empty(); 
        }

        template <typename Comp>
        requires AllComponents<Comp>
        void AssignComponent(const EntityID& ID)
        {
            // todo
        }

        template <typename... Types>
        void RegisterEntity(const ComponentGroupIdentifier& GroupID, const EntityID& ID, Types... Data)
        {
            assert(!EntityMapping.contains(ID));
            auto Set = GetOrCreateSet<Types...>(GroupID);
            Set->Add(ID);
            EntityMapping.emplace(ID, GroupID);
            Set->SetData(ID, Data...);
        }

        
        template <typename... Types>
        /**
         * Adds a range of entities with the given Data
         * Warning: Its important the IDs are not yet added, as
         * it does not single-copy, but rather move the whole mem block!
         * If the assigned IDs are not continuous in dense memory, it will corrupt!
         */
        void RegisterEntities(const ComponentGroupIdentifier& GroupID, const vector<EntityID>& IDs, Types*... Data)
        {
            auto Set = GetOrCreateSet<Types...>(GroupID);
            Set->AddRange(IDs);
            for (const auto& ID : IDs)
            {
                EntityMapping.emplace(ID, GroupID);
            }
            Set->SetDataBlock(
                Set->GetTargetIndex(IDs[0]),
                IDs.size(),
                Data...
            );
        }

        void DeleteEntity(const EntityID& ID)
        {
            assert(EntityMapping.contains(ID));
            auto Set = GetSet(EntityMapping[ID]);
            Set->Remove(ID);
            EntityMapping.erase(ID);
        }

        template <typename ... Components>
        requires AllComponents<Components...>
        void ForEach(EntityAction<Components...>& Action)
        {
            ForEach(Get<Components...>(), Action);
        }

        template <typename ... Components>
        requires AllComponents<Components...>
        void ForEach(const ComponentGroupView& View, EntityAction<Components...>& Action)
        {
            for (const auto& GroupID : View.Groups)
            {
                auto Group = GetSet(*GroupID.get());
                Group->ForEach(Action);
            }
        }
        
    protected:
        void StartServiceInternal() override;
        void ForEachSystem(function<void(const shared_ptr<System>&)> Func) const;

        void StopServiceInternal() override;
        void ResetServiceInternal() override;

        
        map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> GetViewSet() override;
    };
}
