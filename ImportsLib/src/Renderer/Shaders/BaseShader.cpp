#include "BaseShader.h"

#include <filesystem>
#include <glfw/include/GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>


#include "../Camera.h"
#include "../Light.h"
#include "../../Util/ShaderHelper.h"
#include "../Passes/RenderPass.h"
using namespace Util;

void BaseShader::Create()
{
    const wchar_t* VertexShader = L"BASE_VERTEX_SHADER";
    const wchar_t* FragmentShader = L"BASE_FRAGMENT_SHADER";
    CreateInternal(VertexShader, FragmentShader);
}

void BaseShader::CreateInternal(const wchar_t* VShader, const wchar_t* FShader)
{
    std::string VertexCode = ShaderHelper::LoadShaderFromResource(VShader);
    std::string FragmentCode = ShaderHelper::LoadShaderFromResource(FShader);

    unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    vector IDs = {Vertex, Fragment};
    Program = ShaderHelper::CreateProgram(IDs);
    glObjectLabel(GL_PROGRAM, Program, -1, "BaseShaderProgram");
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);

    Transform = glm::mat4(1.0f);
}


void BaseShader::Use(RenderPassType Type)
{
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(Program);
}

void BaseShader::UpdateVars(const shared_ptr<Camera>& Camera, const shared_ptr<Light>& Light) const
{
    float Time = static_cast<float>(glfwGetTime());
    float TimeOffset = (sin(Time) / 2.0f) + 0.5f;
    ShaderHelper::SetUniform1f("Offset", TimeOffset, Program);

    ShaderHelper::SetUniformM4("Transform", Transform, Program);
    ShaderHelper::SetUniformM4("Projection", Camera->Projection, Program);
    ShaderHelper::SetUniformM4("View", Camera->View, Program);
    ShaderHelper::SetUniform3fv("LightPos", Light->Position, Program);
    ShaderHelper::SetUniform3fv("LightDir", Light->GetForward(), Program);
    ShaderHelper::SetUniform3fv("CamPos", Camera->Position, Program);
}

bool BaseShader::SupportsPass(RenderPassType Type)
{
    return Type == RenderPassType::BasePass;
}

void BaseShader::CleanUp() const
{
    glDeleteProgram(Program);
}
