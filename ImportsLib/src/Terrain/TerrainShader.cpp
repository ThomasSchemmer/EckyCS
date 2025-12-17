#include "TerrainShader.h"
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>
#include "../Renderer/Camera.h"
#include "../Renderer/Light.h"
#include "../Util/ShaderHelper.h"
class Light;
using namespace Util;

TTerrain::TerrainShader::TerrainShader()
{
    string VertexCode = ShaderHelper::LoadShaderFromResource(VertexShader);
    string FragmentCode = ShaderHelper::LoadShaderFromResource(FragmentShader);
    string DepthFragmentCode = ShaderHelper::LoadShaderFromResource(DepthFragmentShader);

    unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);
    unsigned int DepthFragment = ShaderHelper::CompileShader(DepthFragmentCode, GL_FRAGMENT_SHADER);

    vector IDs = {Vertex, Fragment};
    Program = ShaderHelper::CreateProgram(IDs);
    vector DepthIDs = {Vertex, DepthFragment};
    DepthProgram = ShaderHelper::CreateProgram(DepthIDs);
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    glDeleteShader(DepthFragment);
    
    Transform = glm::mat4(1.0f);
}

void TTerrain::TerrainShader::Use(RenderPassType Type)
{
    ActiveProgram = Type == RenderPassType::ShadowPass ?
        DepthProgram : Program;
    glUseProgram(ActiveProgram);
}

void TTerrain::TerrainShader::UpdateVars(const TerrainShaderSettings& Settings) const
{
    // where should this be placed in world space?
    ShaderHelper::SetUniform2iv("TexSize", Settings.TexSize, ActiveProgram);
    ShaderHelper::SetUniform3fv("GlobalWorldPos", Settings.GlobalWorldPos, ActiveProgram);
    ShaderHelper::SetUniformM4("Transform", Transform, ActiveProgram);

    // where the user is currently painting
    ShaderHelper::SetUniform3fv("BrushPos", Settings.BrushPos, ActiveProgram);
    ShaderHelper::SetUniform1ui("BrushSize", Settings.BrushSize, ActiveProgram);

    // procedural texturing info
    ShaderHelper::SetUniform3fv("DirtColor", Settings.DirtColor, ActiveProgram);
    ShaderHelper::SetUniform3fv("GrassColor", Settings.GrassColor, ActiveProgram);
    ShaderHelper::SetUniform1f("GrassScale", Settings.GrassScale, ActiveProgram);
    ShaderHelper::SetUniform1f("GrassQuantize", Settings.GrassQuantize, ActiveProgram);

    // shadow pass should be from the lights perspective!
    auto& Projection = Settings.RenderPassType == RenderPassType::ShadowPass ?
        Settings.Light->Projection : Settings.Camera->Projection;
    auto& View = Settings.RenderPassType == RenderPassType::ShadowPass ?
        Settings.Light->View : Settings.Camera->View;

    // rendering info, useful for shadows etc
    ShaderHelper::SetUniformM4("Projection", Projection, ActiveProgram);
    ShaderHelper::SetUniformM4("View", View, ActiveProgram);
    ShaderHelper::SetUniform3fv("LightDir", Settings.Light->GetForward(), ActiveProgram);
    ShaderHelper::SetUniform3fv("LightPos", Settings.Light->Position, ActiveProgram);
    ShaderHelper::SetUniform2fv("LightClip", Settings.Light->ClipPlanes, ActiveProgram);
    ShaderHelper::SetUniformM4("LightProjection", Settings.Light->Projection, ActiveProgram);
    ShaderHelper::SetUniformM4("LightView", Settings.Light->View, ActiveProgram);
    ShaderHelper::SetUniformTexture("ShadowMap", Settings.ShadowMap, 0, ActiveProgram);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Settings.VertexBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Settings.NormalBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Settings.HeightBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, Settings.SelectionBuffer);

}

void TTerrain::TerrainShader::CleanUp() const
{
    glDeleteProgram(Program);
    glDeleteProgram(DepthProgram);
}

