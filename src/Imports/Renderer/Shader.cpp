#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

Shader::Shader()
{
    std::string VertexCode = LoadShader(VertexShaderPath);
    std::string FragmentCode = LoadShader(FragmentShaderPath);

    unsigned int Vertex = CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    Program = CreateProgram(Vertex, Fragment);
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);  
}

void Shader::Use()
{
    glUseProgram(Program);
}

std::string Shader::LoadShader(const std::string& FilePath) {
    std::ifstream ShaderFile;
    std::string ShaderCode;

    ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        auto AbsolutePath = std::filesystem::absolute(FilePath);
        std::cout << "Loading Shader at: " << AbsolutePath << "\n";
        
        for (const auto & entry : std::filesystem::directory_iterator("./src/Shaders/"))
            std::cout << entry.path() << std::endl;
        
        ShaderFile.open(FilePath);
        std::stringstream ShaderStream;
        ShaderStream << ShaderFile.rdbuf();
        ShaderFile.close();
        ShaderCode = ShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_MISSING: " << e.what() << "\n";
    }
    return ShaderCode;
}

unsigned int Shader::CompileShader(const std::string& Code, GLenum Type)
{
    int Success;
    unsigned int ID = glCreateShader(Type);
    const char* SourceCode = Code.c_str();
    glShaderSource(ID, 1, &SourceCode, nullptr);
    glCompileShader(ID);
    glGetShaderiv(ID, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        char InfoLog[512];
        glGetShaderInfoLog(ID, 512, nullptr, InfoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << InfoLog << "\n";
    }
    return ID;
}

unsigned int Shader::CreateProgram(unsigned int Vertex, unsigned int Fragment)
{
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, Vertex);
    glAttachShader(ID, Fragment);
    glLinkProgram(ID);

    int Success;
    glGetProgramiv(ID, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        char InfoLog[512];
        glGetShaderInfoLog(ID, 512, nullptr, InfoLog);
        std::cout << "ERROR::SHADER::PROGRAM_CREATION_FAILED\n" << InfoLog << "\n";
    }
    
    return ID;
}
