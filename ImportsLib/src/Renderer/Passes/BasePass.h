#pragma once
#include <glew/include/GL/glew.h>
#include <GLFW/glfw3.h>

#include "RenderPass.h"

class BasePass : public RenderPass
{
public:
    void Create(GLFWwindow* Window) override
    {
        Type = RenderPassType::BasePass; 
        bCreateFrameBuffer = false;
        glfwGetFramebufferSize(Window, &Width, &Height);
        RenderPass::Create(Window);
    }
};