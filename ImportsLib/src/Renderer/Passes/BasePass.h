#pragma once
#include <glew/include/GL/glew.h>
#include <GLFW/glfw3.h>

#include "RenderPass.h"

/** RenderPass for all regularly displayed (ie non-transparent) objects in the scene
 * Has information from shadow and depth pass
 * Followed by transparent pass
 */
class BasePass : public RenderPass
{
public:
    void Create(GLFWwindow* Window, Renderer* Renderer) override;
};