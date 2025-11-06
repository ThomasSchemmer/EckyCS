#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <glfw/include/GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "Camera.h"
#include "./stb/stb_image.h"


Shader::Shader()
{
    std::string VertexCode = LoadShader(VertexShaderPath);
    std::string FragmentCode = LoadShader(FragmentShaderPath);

    unsigned int Vertex = CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    Program = CreateProgram(Vertex, Fragment);
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);

    ContainerTex = CreateTexture("./ImportsLib/Textures/container.jpg", GL_RGB);
    CornTex = CreateTexture("./ImportsLib/Textures/Corntex.png", GL_RGBA);
    Transform = glm::mat4(1.0f);
}

void Shader::Use()
{
    glUseProgram(Program);
}

void Shader::UpdateVars(const shared_ptr<Camera>& Camera)
{

    float Time = static_cast<float>(glfwGetTime());
    float TimeOffset = (sin(Time) / 2.0f) + 0.5f;
    SetUniform1f("Offset", TimeOffset);

    SetUniformTexture("ContainerTex", ContainerTex, 0);
    SetUniformTexture("CornTex", CornTex, 1);

    Transform = glm::mat4(1.0f);
    
    SetUniformM4("Transform", Transform);
    SetUniformM4("Projection", Camera->Projection);
    SetUniformM4("View", Camera->View);
}

std::string Shader::LoadShader(const std::string& FilePath) {
    std::ifstream ShaderFile;
    std::string ShaderCode;

    ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        auto AbsolutePath = std::filesystem::absolute(FilePath);
        std::cout << "Loading Shader at: " << AbsolutePath << "\n";
        
        for (const auto & entry : std::filesystem::directory_iterator("./src/Shaders/"))
            std::cout << entry.path() << "\n";
        
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

unsigned int Shader::CreateTexture(const string& FilePath, GLint Format)
{
    int Width, Height, CountChannels;
    unsigned char* Data = stbi_load(FilePath.c_str(), &Width, &Height, &CountChannels, 0);
    if (!Data)
    {
        std::cerr << "ERROR::SHADER::TEX_FILE_MISSING: " << FilePath << "\n";
    }

    unsigned int ID;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(Data);
    return ID;
}

void Shader::SetUniform1f(const string& UniformName, float Value) const
{
    const int ID = glGetUniformLocation(Program, UniformName.c_str());
    glUniform1f(ID, Value);
}

void Shader::SetUniformM4(const string& UniformName, const glm::mat4& Value) const
{
    const int ID = glGetUniformLocation(Program, UniformName.c_str());
    glUniformMatrix4fv(ID, 1, GL_FALSE, value_ptr(Value));
}

void Shader::SetUniformTexture(const string& UniformName, unsigned int TextureID, GLint Slot) const
{
    const int ID = glGetUniformLocation(Program, UniformName.c_str());
    glUniform1i(ID, Slot);
    glActiveTexture(GL_TEXTURE0 + Slot);
    glBindTexture(GL_TEXTURE_2D, TextureID);
}
