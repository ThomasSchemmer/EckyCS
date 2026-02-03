#pragma once
#include "../EckyCS/Systems/Rendering/RenderData.h"

class ObjectRenderData : public RenderData
{
public:
    ObjectRenderData() = default;
    const wchar_t* GetResourcePath() const override
    {
        return Str;
    }

    // todo: load dynamically for multiple objects
    const wchar_t* Str = L"RESOURCE_STONES";
};
