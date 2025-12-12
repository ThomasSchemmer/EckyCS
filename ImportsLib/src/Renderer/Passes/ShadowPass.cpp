#include "ShadowPass.h"

#include "../../Util/ShaderHelper.h"
using namespace Util;

void ShadowPass::Create(GLFWwindow* Window) 
{
    Type = RenderPassType::ShadowPass; 
    Width = 1024;
    Height = 1024;
    ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    DepthTex = CreateDepthTexture(Width, Height, GL_NEAREST);
    ColorTex = CreateColorTexture(Width, Height, GL_LINEAR);
    Name = "ShadowPass";
    RenderPass::Create(Window);
    TempOut = CreateColorTexture(Width, Height, GL_LINEAR);
    
    BlurTexCompute = ShaderHelper::CreateProgram({BlurTexComputeID});
}


void ShadowPass::OnAfterRender()
{
    glUseProgram(BlurTexCompute);
    glBindImageTexture(0, ColorTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
    glBindImageTexture(1, TempOut, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
    ShaderHelper::SetUniform2iv("Axis", glm::ivec2(1, 0), BlurTexCompute);
    glDispatchCompute(Width / 8, Height / 8, 1);
    ShaderHelper::SetUniform2iv("Axis", glm::ivec2(0, 1), BlurTexCompute);
    glDispatchCompute(Width / 8, Height / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void ShadowPass::CleanUp() const
{
    RenderPass::CleanUp();
    glDeleteProgram(BlurTexCompute);
}
