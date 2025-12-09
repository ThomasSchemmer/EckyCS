#pragma once
#include <glew/include/GL/glew.h>
#include <GLFW/glfw3.h>

enum class RenderPassType : uint8_t
{
    Invalid = 0,
    BasePass = 1,
    ShadowPass = 2,
};

/**
 * A collection of graphic calls, usually with a distinct target
 * e.g.: render all shadows
 */
class RenderPass
{
public:
    RenderPassType Type;
    int Width = 1920, Height = 1080;
    GLuint DepthTex = 0;
    GLuint ColorTex = 0;
    GLuint FBO = 0;
    // should only be touched by BasePass
    bool bCreateFrameBuffer = true;

    virtual void Create(GLFWwindow*);
    virtual void Use() const;
    virtual void UnUse() const {}
    void CleanUp() const;

    RenderPass() = default;
    virtual ~RenderPass() = default;

protected:
    GLbitfield ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    static GLuint CreateDepthTexture(int Width, int Height);
};
