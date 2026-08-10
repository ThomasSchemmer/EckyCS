#pragma once
#include "RenderPass.h"

class TransparentPass : public RenderPass
{
public:
    void Create(GLFWwindow* Window, Renderer* Renderer) override
    {
        Type = RenderPassType::TransparentPass;
        FBOTarget = FrameBufferTarget::Base;
        SetDimensions(Window);
        Name = "TransparentPass";
        RenderPass::Create(Window, Renderer);
    }

    void Use() const override
    {
        RenderPass::Use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // disable depth writes
        glDepthMask(GL_FALSE);
        // keep depth testing
        glEnable(GL_DEPTH_TEST); 
    }

    void UnUse() const override
    {
        RenderPass::UnUse();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    
};
