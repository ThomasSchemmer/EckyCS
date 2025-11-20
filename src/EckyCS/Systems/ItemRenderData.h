#pragma once
#include "EckyCS/Systems/Rendering/RenderData.h"

using namespace EckyCS;

class ItemRenderData : public RenderData
{
public:
    ItemRenderData() = default;
    ~ItemRenderData() override = default;
    void Create(size_t count) override;

private:
    unsigned int ItemBuffer;
};
