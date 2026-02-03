#pragma once
#include "../EckyCS/Components/Base/TransformComponent.h"
#include "../EckyCS/Entities/Entity.h"
#include "GL/glew.h"

using namespace EckyCS;

/** Prolly needs an identifier component too */
class Object : public Entity<TransformComponent>
{ 
};
