#include "RenderPass.h"

#include <iostream>

void RenderPass::Create(GLFWwindow*)
{
    if (!bCreateFrameBuffer)
        return;
    
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    if (DepthTex)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTex, 0);
    }
    if (ColorTex)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorTex, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    
    glObjectLabel(GL_FRAMEBUFFER, FBO, -1, Name.data());
    auto Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::RENDERPASS::USE_FAILED\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass::Use() const
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, Name.data());
    glViewport(0, 0, Width, Height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(ClearFlags);
}

void RenderPass::UnUse() const
{
    glPopDebugGroup();
}

void RenderPass::CleanUp() const
{
    if (DepthTex != 0) glDeleteTextures(1, &DepthTex);
    if (ColorTex != 0) glDeleteTextures(1, &ColorTex);
    if (FBO != 0) glDeleteFramebuffers(1, &FBO);
}

GLuint RenderPass::CreateDepthTexture(int Width, int Height, GLint DepthSamplingMethod)
{
    GLuint Tex;
    glGenTextures(1, &Tex);
    glBindTexture(GL_TEXTURE_2D, Tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, DepthSamplingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, DepthSamplingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    return Tex;
}

GLuint RenderPass::CreateColorTexture(int Width, int Height, GLint ColorSamplingMethod)
{
    GLuint Tex;
    glGenTextures(1, &Tex);
    glBindTexture(GL_TEXTURE_2D, Tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, Width, Height, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ColorSamplingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ColorSamplingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    return Tex;
}
