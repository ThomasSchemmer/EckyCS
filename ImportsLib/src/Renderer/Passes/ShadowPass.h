#pragma once
#include "RenderPass.h"

class ShadowPass : public RenderPass
{
public:
    
    void Create(GLFWwindow* Window) override
    {
        Type = RenderPassType::ShadowPass; 
        Width = 1024;
        Height = 1024;
        ClearFlags = GL_DEPTH_BUFFER_BIT;
        DepthTex = CreateDepthTexture(Width, Height);
        RenderPass::Create(Window);
    }

    void Use() const override
    {
        RenderPass::Use();
        //glCullFace(GL_FRONT);
    }

    void UnUse() const override
    {
        RenderPass::UnUse();
        //glCullFace(GL_BACK);
    }

};