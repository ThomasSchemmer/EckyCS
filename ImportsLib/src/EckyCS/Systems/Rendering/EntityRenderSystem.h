#pragma once
#include <map>

#include "GeometryProvider.h"
#include "RenderData.h"
#include "../System.h"
#include "../../Util/EckyCSHeader.h"
#include "../../Entities/Entity.h"

namespace EckyCS
{
    class ExplicitRenderData;
    class RenderData;
    class ComponentGroupIdentifier;

    /**
     * Mini class that acts as a public header for the actual
     * templated render class. Useful, since we can't store templated ptrs!
     */
    class BaseRenderSystem : public System
    {
    public:
        virtual void Render() const {}
        virtual bool SupportsRenderPass(RenderPassType Type) const = 0;
    };
    
    template <typename EntityType, typename RenderDataType, typename GeometryProviderType>
    requires HasRequiredComponents<EntityType> &&
        is_base_of_v<RenderData, RenderDataType> &&
        is_base_of_v<GeometryProvider, GeometryProviderType>
    /**
     * Handles registering and rendering RenderData, which in turn
     * describe how Entities should be rendered
     */
    class EntityRenderSystem : public BaseRenderSystem
    {
    protected:
        map<ComponentGroupIdentifier, RenderDataType> Datas;

    public:
        // results in a tuple<CompA, CompB, ...>
        using RequiredTuple = typename EntityType::RequiredComponents;
        // results in View<CompA, CompB, ...>
        using RequiredView = typename UnpackTuple<RequiredTuple>::template apply<View>;
        
        bool Register(ComponentGroupIdentifier GroupID, size_t Count, RequiredView& Data)
        {
            if (Datas.contains(GroupID) && Datas[GroupID].Count < Count)
            {
                Datas.erase(GroupID);
            }
            if (!Datas.contains(GroupID))
            {
                auto ProviderPtr = make_shared<GeometryProviderType>();
                Datas[GroupID] = RenderDataType();
                Datas[GroupID].Create(Count, ProviderPtr);
            }

            if constexpr (std::is_base_of_v<ExplicitRenderData, RenderDataType>)
            {
                static_cast<ExplicitRenderData*>(&Datas[GroupID])->UpdateBuffers(GroupID, Count, Data);
            }
            
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