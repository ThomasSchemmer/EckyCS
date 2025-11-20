#pragma once
#include "../Components/ItemComponent.h"
#include "EckyCS/Components/Base/TransformComponent.h"
#include "EckyCS/Entities/Entity.h"

using namespace EckyCS;
class PlantTest : public Entity<ItemComponent, TransformComponent, Component>
{
    
};
