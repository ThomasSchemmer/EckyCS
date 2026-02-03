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
    void Create(size_t count, const shared_ptr<GeometryProvider>& Provider) override;
    const wchar_t* GetResourcePath() const override { return ResourcePath; }

private:
    unsigned int ItemBuffer;
    const wchar_t* ResourcePath = L"";
};
