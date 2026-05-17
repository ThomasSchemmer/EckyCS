#pragma once
#include "RenderPass.h"

class TransparentPass : public RenderPass
{
public:
    void Create(GLFWwindow* Window) override
    {
        Type = RenderPassType::TransparentPass; 
        bCreateFrameBuffer = false;
        glfwGetFramebufferSize(Window, &Width, &Height);
        Name = "TransparentPass";
        RenderPass::Create(Window);
    }

    void Use() const override
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // disable depth writes
        glDepthMask(GL_FALSE);
        // keep depth testing
        glEnable(GL_DEPTH_TEST); 
    }

    void UnUse() const override
    {
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    
};
