#pragma once
#include "RenderPass.h"

/** Renderpass that handles pixelation (aka downsampling) of incoming image*/
class PostProcessingPass : public RenderPass
{
public:
    PostProcessingPass(GLuint ColorTex);
    void Create(GLFWwindow*, Renderer* Renderer) override;
    void Use() const override;
    void UnUse() const override; 
    
};
