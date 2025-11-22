#pragma once
#include <map>

#include "RenderData.h"
#include "../System.h"
#include "../../Util/EckyCSHeader.h"
#include "../../Entities/Entity.h"

namespace EckyCS
{
    class RenderData;
    class ComponentGroupIdentifier;

    class BaseRenderSystem : public System
    {
    public:
        virtual void Render() const {}
    };
    
    template <typename EntityType, typename RenderDataType>
    requires HasRequiredComponents<EntityType> &&
    is_base_of_v<RenderData, RenderDataType>
    class EntityRenderSystem : public BaseRenderSystem
    {
    private:
        map<ComponentGroupIdentifier, RenderDataType> Datas;

        // results in a tuple<CompA, CompB, ...>
        using RequiredTuple = typename EntityType::RequiredComponents;
        // results in View<CompA, CompB, ...>
        using RequiredView = typename UnpackTuple<RequiredTuple>::template apply<View>;
        
    public:
        bool Register(ComponentGroupIdentifier GroupID, size_t Count, RequiredView& Data)
        {
            if (Datas.contains(GroupID) && Datas[GroupID].Count < Count)
            {
                Datas.erase(GroupID);
            }
            if (!Datas.contains(GroupID))
            {
                Datas[GroupID] = RenderDataType();
                Datas[GroupID].Create(Count);
            }

            Datas[GroupID].UpdateBuffers(GroupID, Count, Data);
            return true;
        }

        void Render() const override
        {
            for (const auto& Pairs : Datas)
            {
                Pairs.second.Render();
            }
        }

    };

}