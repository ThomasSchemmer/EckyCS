#pragma once
#include "EckyCS/Systems/Rendering/RenderData.h"
#include "EckyCS/Systems/Rendering/SpriteGeometryProvider.h"

using namespace EckyCS;

/**
 * Converts and stores Item data into renderable sprite info
 */
class ItemRenderData : public RenderData
{
public:
    ItemRenderData() = default;
    virtual ~ItemRenderData() override = default;
    void CreateInternal(const wchar_t* Path, void* Data, 
        size_t InCount, const shared_ptr<GeometryProvider>& Provider) override;

    static constexpr ModelDefinition Model{
        .Name = L"",
        .MeshIndex = 0,
        .PrimitiveIndex = 0
    };
    
private:
    unsigned int ItemBuffer;
    const wchar_t* ResourcePath = L"";
};
