#include "DepthPrePass.h"

void DepthPrePass::Create(GLFWwindow* Window, Renderer* Renderer)
{
    Type = RenderPassType::DepthPrePass;
    FBOTarget = FrameBufferTarget::Depth;
    SetDimensions(Window);
    ClearFlags = GL_DEPTH_BUFFER_BIT;
    DepthTex = CreateDepthTexture(Width, Height, GL_NEAREST);
    Name = "DepthPrePass";
    RenderPass::Create(Window, Renderer);
}
