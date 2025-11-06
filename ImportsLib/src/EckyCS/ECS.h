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
    class ECS : public GameService, public IComponentGroupProvider
    {
        map<TypeInfo, vector<shared_ptr<System>>> Systems;
        map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> Sets;
        map<EntityID, ComponentGroupIdentifier> EntityMapping;
        
        ComponentGroupIdentifier EmptyGroup;
        
    public:
        void Update() override;
        void FixedUpdate() override;
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

            System->StartSystem();
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
                T* Ptr = dynamic_pointer_cast<T>(Tuple.second[0]);
                if (!Ptr)
                    continue;

                OutList.insert(OutList.end(), Tuple.second.begin(), Tuple.second.end());
            }

            return !OutList.empty(); 
        }

        template <typename Comp>
        requires AllComponents<Comp>
        void AssignComponent(const EntityID& ID){}

        template <typename... Types>
        void RegisterEntity(const ComponentGroupIdentifier& GroupID, const EntityID& ID, Types... Data)
        {
            assert(!EntityMapping.contains(ID));
            auto Set = GetOrCreateSet<Types...>(GroupID);
            Set->Add(ID);
            EntityMapping.emplace(ID, GroupID);
            Set->SetData(ID, Data...);
        }

        void DeleteEntity(const EntityID& ID)
        {
            assert(EntityMapping.contains(ID));
            auto Set = GetSet(EntityMapping[ID]);
            Set->Remove(ID);
            EntityMapping.erase(ID);
        }
        
    protected:
        void StartServiceInternal() override;
        void ForEachSystem(function<void(const shared_ptr<System>&)> Func) const;

        void StopServiceInternal() override;
        void ResetServiceInternal() override;

        
        map<ComponentGroupIdentifier, shared_ptr<ISparseSet>> GetViewSet() override;
    };
}
