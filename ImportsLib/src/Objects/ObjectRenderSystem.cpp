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
    EntityGenerator::TryCreate<Object>(GroupID, ID);
}
