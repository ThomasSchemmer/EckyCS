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
    CreateProgram(VShader, FShader, "BaseShaderProgram", Program);
    Transform = glm::mat4(1.0f);
}

void BaseShader::CreateProgram(const wchar_t* VShader, const wchar_t* FShader, const string& Name, GLuint& Result) const
{
    std::string VertexCode = ShaderHelper::LoadShaderFromResource(VShader);
    std::string FragmentCode = ShaderHelper::LoadShaderFromResource(FShader);

    unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    vector IDs = {Vertex, Fragment};
    Result = ShaderHelper::CreateProgram(IDs);
    glObjectLabel(GL_PROGRAM, Program, -1, Name.c_str());
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
}


void BaseShader::Use(RenderPassType Type, void* UserData)
{
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(Program);
}

void BaseShader::UpdateVars(const shared_ptr<Camera>& Camera, const shared_ptr<Light>& Light, RenderPassType Type) const
{
    float Time = static_cast<float>(glfwGetTime());
    float TimeOffset = (sin(Time) / 2.0f) + 0.5f;
    ShaderHelper::SetUniform1f("Offset", TimeOffset, Program);

    ShaderHelper::SetUniformM4("Transform", Transform, Program);
    ShaderHelper::SetUniformM4("Projection", Type != RenderPassType::ShadowPass ? Camera->Projection : Light->Projection, Program);
    ShaderHelper::SetUniformM4("View", Type != RenderPassType::ShadowPass ? Camera->View : Light->View, Program);
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
