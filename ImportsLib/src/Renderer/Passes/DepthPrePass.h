#pragma once
#include "RenderPass.h"

/**
 * Renders depth buffer for terrain etc so that later stages can query it
 * Used for now only in water depth check
 * TODO: add hi-z path?
 */
class DepthPrePass : public RenderPass
{
public:
    void Create(GLFWwindow* Window) override;
    
    
};
