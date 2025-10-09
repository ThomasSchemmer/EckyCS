#pragma once
#include <string>
#include <GL/glew.h>

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
