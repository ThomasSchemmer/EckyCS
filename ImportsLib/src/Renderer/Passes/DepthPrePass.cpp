#include "DepthPrePass.h"

void DepthPrePass::Create(GLFWwindow* Window)
{
    Type = RenderPassType::DepthPrePass;
    glfwGetFramebufferSize(Window, &Width, &Height);
    ClearFlags = GL_DEPTH_BUFFER_BIT;
    DepthTex = CreateDepthTexture(Width, Height, GL_NEAREST);
    Name = "DepthPrePass";
    RenderPass::Create(Window);
}
