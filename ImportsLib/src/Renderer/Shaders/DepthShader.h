#pragma once
#include "BaseShader.h"

/** Class linking the depth shader glsl files */
class DepthShader : public BaseShader
{
public:
    void Create() override;
    bool SupportsPass(RenderPassType Type) override;
};
