#pragma once
#include "ItemRenderData.h"
#include "../ItemEntity.h"
#include "EckyCS/Systems/Rendering/EntityRenderSystem.h"
#include "GameService/GameServiceDelegate.h"
#include "../Components/ItemComponent.h"
#include "../ext/imgui/imgui.h"

using namespace EckyCS;
class ItemRenderSystem : public EntityRenderSystem<ItemEntity, ItemRenderData>
{
public:
    void Tick(float Delta) override
    {
        if (!Ecs)
            return;

        TotalCount = 0;
        EntityAction Action = [this](ComponentGroupIdentifier GroupID, size_t Count, View<ItemComponent, TransformComponent>& Data) -> bool {
            TotalCount += Count; 
            return this->Register(GroupID, Count, Data);
        };
        Ecs->ForEach<ItemComponent, TransformComponent>(Action);
    }

    void OnDrawGizmos() override
    {        
        ImGui::Begin("ItemRenderSystem");
        ImGui::Text("Entities: %d", TotalCount);
        ImGui::Text("Size (mb): %f", TotalCount * sizeof(ItemEntity) / 1024.0 / 1024.0);
        ImGui::End();
    }

    void StartSystem(shared_ptr<ECS>& Ptr) override
    {
        Ecs = Ptr;
    }
    
private:
    shared_ptr<ECS> Ecs;
    size_t TotalCount = 0;
};
