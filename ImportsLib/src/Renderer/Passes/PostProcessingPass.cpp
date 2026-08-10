#include "PostProcessingPass.h"
#include "../Renderer.h"

PostProcessingPass::PostProcessingPass(GLuint ColorTexPtr) 
{
    ColorTex = ColorTexPtr;
}

void PostProcessingPass::Create(GLFWwindow* Window, Renderer* Renderer)
{
    Type = RenderPassType::PostProcessingPass;
    bIsDownSampled = false;
    SetDimensions(Window);
    ColorTex = CreateColorTexture(Width, Height, GL_LINEAR);
    Name = "PostProcessingPass";
    RenderPass::Create(Window, Renderer);
}

void PostProcessingPass::Use() const
{
    RenderPass::Use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthFunc(GL_ALWAYS);
}

void PostProcessingPass::UnUse() const
{
    RenderPass::UnUse();
    glDepthFunc(GL_LESS);
}


