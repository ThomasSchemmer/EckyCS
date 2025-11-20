#pragma once
#include "../Components/ItemComponent.h"
#include "EckyCS/Components/Base/TransformComponent.h"
#include "EckyCS/Entities/Entity.h"

using namespace EckyCS;
class Plant : public Entity<ItemComponent, TransformComponent>
{
    
};
