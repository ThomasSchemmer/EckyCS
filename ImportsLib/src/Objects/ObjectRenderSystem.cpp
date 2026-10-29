#include "ObjectRenderSystem.h"
#include "../EckyCS/ECS.h"
#include "../EckyCS/Entities/EntityGenerator.h"

void ObjectRenderSystem::Tick(float Delta)
{
    if (!EcsPtr)
        return;

    EntityAction Action = [this](ComponentGroupIdentifier GroupID, size_t Count, View<TransformComponent>& Data) -> bool {
        return this->Register(GroupID, Count, Data);
    };
    EcsPtr->ForEach<TransformComponent>(Action);
}

void ObjectRenderSystem::StartSystem(shared_ptr<ECS>& Ecs)
{
    EcsPtr = Ecs;
    ComponentGroupIdentifier GroupID;
    EntityID ID;
    TransformComponent Component;
    Component.PosX = 30;
    Component.PosY = 0;
    Component.PosZ = 30;
    Component.SetRotation(glm::radians(15.0), 0, 0);
    EntityGenerator::TryCreate<Object>(GroupID, ID, Component);
}
