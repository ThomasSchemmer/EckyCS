#pragma once
#include <glew/include/GL/glew.h>

enum class RenderPassType : unsigned char;

/** Interface for any shader, mostly used to force pass support */
class Shader
{
public:
    Shader() = default;
    virtual ~Shader() = default;
    
    /** Returns true for any pass that it supports */
    // TODO: register into lists instead of perma-querying
    virtual bool SupportsPass(RenderPassType Type) = 0;

    /** Delayed constructor */
    virtual void Create() = 0;

    /** Applies the shader for any following rendering */
    virtual void Use(RenderPassType Type) = 0;

    /** Destroys any created OGL vars*/
    virtual void CleanUp() const = 0;

    /** The main program (usually vs/fs) that the shader uses */
    GLuint Program;
};
