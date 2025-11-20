#pragma once
#include "EckyCS/Components/Base/TransformComponent.h"
#include "EckyCS/Entities/Entity.h"
#include "EckyCS/Util/EckyCSHeader.h"
#include "../Components/ItemComponent.h"

using namespace EckyCS;
class ItemEntity : public Entity<ItemComponent, TransformComponent>
{
    
};
