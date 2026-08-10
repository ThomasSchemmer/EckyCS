#pragma once

#include <string>
#include <glew/include/GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/vec4.hpp"

class Renderer;

enum class RenderPassType : uint8_t
{
    Invalid = 0,
    BasePass = 1,
    ShadowPass = 2,
    DepthPrePass = 3,
    TransparentPass = 4,
    PostProcessingPass = 5
};

enum class FrameBufferTarget : uint8_t
{
    Default = 0,
    Depth = 1,
    Shadow = 2,
    // contains opaque and transparent color
    Base = 3,
    PostProcessing = 4,
};

inline const char* ToString(RenderPassType type)
{
    switch(type)
    {
        case RenderPassType::BasePass:              return "Base";
        case RenderPassType::ShadowPass:            return "Shadow";
        case RenderPassType::DepthPrePass:          return "DepthPrePass";
        case RenderPassType::TransparentPass:       return "Transparent";
        case RenderPassType::PostProcessingPass:    return "PostProcessing";
        default:                                    return "INVALID";
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
    FrameBufferTarget FBOTarget;
    int Width = 1920, Height = 1080;
    GLuint DepthTex = 0;
    GLuint ColorTex = 0;
    GLuint FBO = 0;
    std::string Name;
    bool bIsDownSampled = true;

    virtual void Create(GLFWwindow*, Renderer* Renderer);
    virtual void Use() const;
    virtual void UnUse() const;
    virtual void CleanUp() const;
    virtual void SetDimensions(GLFWwindow* Window);
    
    /** Run anything that should be done after everything has been rendered */
    virtual void OnAfterRender() {}

    RenderPass() = default;
    virtual ~RenderPass() = default;

protected:
    glm::vec4 ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    GLbitfield ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    static GLuint CreateDepthTexture(int Width, int Height, GLint DepthSamplingMethod);
    static GLuint CreateColorTexture(int Width, int Height, GLint ColorSamplingMethod, bool bUseRGB = true);
};
