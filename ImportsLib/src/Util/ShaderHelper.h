#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
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

        static unsigned int CompileShader(const string& Code, GLenum Type);

        static unsigned int CreateProgram(const vector<unsigned int>& ShaderIDs);

        static unsigned int CreateTexture(const string& FilePath, GLint Format);

        static void SetUniform1f(const string& UniformName, float Value, unsigned int Program);

        static void SetUniformM4(const string& UniformName, const glm::mat4& Value, unsigned int Program);

        static void SetUniform3fv(const string& UniformName, const glm::vec3& Value, unsigned int Program) ;

        static void SetUniformTexture(const string& UniformName, unsigned int TextureID, GLint Slot, unsigned int Program);

    };
}
