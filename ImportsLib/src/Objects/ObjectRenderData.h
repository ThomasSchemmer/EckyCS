#pragma once
#include "../EckyCS/Systems/Rendering/RenderData.h"

class ObjectRenderData : public RenderData
{
public:
    ObjectRenderData() = default;
    
    static constexpr ModelDefinition Model{
        .Name = L"RESOURCE_TREE",
        .MeshIndex = 0,
        .PrimitiveIndex = 0
    };
};
