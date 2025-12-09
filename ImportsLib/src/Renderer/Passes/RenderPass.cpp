#include "RenderPass.h"

#include <iostream>

void RenderPass::Create(GLFWwindow*)
{
    if (!bCreateFrameBuffer)
        return;
    
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTex, 0);
    glDrawBuffer(ColorTex);
    glReadBuffer(ColorTex);
    
    auto Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::RENDERPASS::USE_FAILED\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass::Use() const
{
    glViewport(0, 0, Width, Height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(ClearFlags);
}

void RenderPass::CleanUp() const
{
    if (DepthTex != 0) glDeleteTextures(1, &DepthTex);
    if (ColorTex != 0) glDeleteTextures(1, &ColorTex);
    if (FBO != 0) glDeleteFramebuffers(1, &FBO);
}

GLuint RenderPass::CreateDepthTexture(int Width, int Height)
{
    GLuint Tex;
    glGenTextures(1, &Tex);
    glBindTexture(GL_TEXTURE_2D, Tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    return Tex;
}
