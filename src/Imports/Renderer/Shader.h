#pragma once
#include <string>
#include <GL/glew.h>

/**
 * Class changing the actual display of elements on the screen
 * Contains links to loaded OpenGL structs 
 */
class Shader
{
public:
    unsigned int Program;
    Shader();

    void Use();

private:
    static std::string LoadShader(const std::string& FilePath);
    static unsigned int CompileShader(const std::string& Code, GLenum Type);
    static unsigned int CreateProgram(unsigned int Vertex, unsigned int Fragment);

    std::string VertexShaderPath = "./src/Shaders/BaseVertexShader.vert";
    std::string FragmentShaderPath = "./src/Shaders/BaseFragmentShader.frag";
    
};
