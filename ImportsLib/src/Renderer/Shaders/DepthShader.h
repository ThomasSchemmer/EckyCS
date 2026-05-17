#pragma once
#include "BaseShader.h"

class DepthShader : public BaseShader
{
public:
    void Create() override;
    bool SupportsPass(RenderPassType Type) override;
};
