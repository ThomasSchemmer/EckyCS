#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <glew/include/GL/glew.h>
#include "GLFW/include/GLFW/glfw3.h"
#include <glm/gtc/type_ptr.hpp>

namespace Util
{
    using namespace std;

    /**
     * Helper class that provides easy access for
     * reading & creating shaders
     */
    class ShaderHelper
    {
    public:
        static string LoadShader(const string& FilePath);
        static string LoadShaderFromResource(const wchar_t* Name);

        static unsigned int CompileShader(const string& Code, GLenum Type);

        static unsigned int CreateProgram(const vector<const wchar_t*>& Resources);
        static unsigned int CreateProgram(const vector<unsigned int>& ShaderIDs);
        static unsigned int CreateTexture(const string& FilePath, GLint Format);

        static void SetUniform1f(const string& UniformName, float Value, unsigned int Program);
        static void SetUniform1ui(const string& UniformName, unsigned int Value, unsigned int Program);
        static void SetUniformM4(const string& UniformName, const glm::mat4& Value, unsigned int Program);
        static void SetUniform3fv(const string& UniformName, const glm::vec3& Value, unsigned int Program) ;
        static void SetUniform2fv(const string& UniformName, const glm::vec2& Value, unsigned int Program) ;
        static void SetUniform3iv(const string& UniformName, const glm::ivec3& Value, unsigned int Program) ;
        static void SetUniform2iv(const string& UniformName, const glm::ivec2& Value, unsigned int Program) ;
        static void SetUniformTexture(const string& UniformName, unsigned int TextureID, GLint Slot, unsigned int Program);

        static void ResetBufferCounter(GLuint Buffer);
        static GLsizei ReadBufferCount(GLuint Buffer);

    };
}
