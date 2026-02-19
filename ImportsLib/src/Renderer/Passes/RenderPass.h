#pragma once
#include <string>
#include <glew/include/GL/glew.h>
#include <GLFW/glfw3.h>

enum class RenderPassType : uint8_t
{
    Invalid = 0,
    BasePass = 1,
    ShadowPass = 2,
    DepthPrePass = 3,
};

inline const char* ToString(RenderPassType type)
{
    switch(type)
    {
    case RenderPassType::BasePass:      return "Base";
    case RenderPassType::ShadowPass:    return "Shadow";
    case RenderPassType::DepthPrePass:    return "DepthPrePass";
    default:                            return "INVALID";
    }
}

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
    std::string Name;
    // should only be touched by BasePass
    bool bCreateFrameBuffer = true;

    virtual void Create(GLFWwindow*);
    virtual void Use() const;
    virtual void UnUse() const;
    virtual void CleanUp() const;
    
    /** Run anything that should be done after everything has been rendered */
    virtual void OnAfterRender() {}

    RenderPass() = default;
    virtual ~RenderPass() = default;

protected:
    GLbitfield ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    static GLuint CreateDepthTexture(int Width, int Height, GLint DepthSamplingMethod);
    static GLuint CreateColorTexture(int Width, int Height, GLint ColorSamplingMethod);
};
