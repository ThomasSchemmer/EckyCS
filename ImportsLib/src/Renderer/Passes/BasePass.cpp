#include "BasePass.h"

void BasePass::Create(GLFWwindow* Window, Renderer* Renderer, GLuint InDepthTex)
{
    Type = RenderPassType::BasePass;
    FBOTarget = FrameBufferTarget::Base;
    SetDimensions(Window);
    ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ColorTex = CreateColorTexture(Width, Height, GL_NEAREST);
    DepthTex = InDepthTex;
    Name = "BasePass";
    RenderPass::Create(Window, Renderer);
}