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

    unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    vector IDs = {Vertex, Fragment};
    Program = ShaderHelper::CreateProgram(IDs);
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    
    Transform = glm::mat4(1.0f);
}

void TTerrain::TerrainShader::Use() const
{
    glUseProgram(Program);
}

void TTerrain::TerrainShader::UpdateVars(const TerrainShaderSettings& Settings) const
{
    // where should this be placed in world space?
    ShaderHelper::SetUniform2iv("TexSize", Settings.TexSize, Program);
    ShaderHelper::SetUniform2iv("WorldSize", Settings.WorldSize, Program);
    ShaderHelper::SetUniform3fv("GlobalWorldPos", Settings.GlobalWorldPos, Program);
    ShaderHelper::SetUniformM4("Transform", Transform, Program);

    // where the user is currently painting
    ShaderHelper::SetUniform3fv("BrushPos", Settings.BrushPos, Program);
    ShaderHelper::SetUniform1ui("BrushSize", Settings.BrushSize, Program);

    // procedural texturing info
    ShaderHelper::SetUniform3fv("DirtColor", Settings.DirtColor, Program);
    ShaderHelper::SetUniform3fv("GrassColor", Settings.GrassColor, Program);
    ShaderHelper::SetUniform1f("GrassScale", Settings.GrassScale, Program);
    ShaderHelper::SetUniform1f("GrassQuantize", Settings.GrassQuantize, Program);

    // shadow pass should be from the lights perspective!
    auto& Projection = Settings.RenderPassType == RenderPassType::ShadowPass ?
        Settings.Light->Projection : Settings.Camera->Projection;
    auto& View = Settings.RenderPassType == RenderPassType::ShadowPass ?
        Settings.Light->View : Settings.Camera->View;

    // rendering info, useful for shadows etc
    ShaderHelper::SetUniformM4("Projection", Projection, Program);
    ShaderHelper::SetUniformM4("View", View, Program);
    ShaderHelper::SetUniform3fv("LightDir", Settings.Light->GetForward(), Program);
    ShaderHelper::SetUniform3fv("LightPos", Settings.Light->Position, Program);
    ShaderHelper::SetUniform2fv("LightClip", Settings.Light->ClipPlanes, Program);
    ShaderHelper::SetUniformM4("LightProjection", Settings.Light->Projection, Program);
    ShaderHelper::SetUniformM4("LightView", Settings.Light->View, Program);
    ShaderHelper::SetUniformTexture("ShadowMap", Settings.ShadowMap, 0, Program);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Settings.VertexBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Settings.NormalBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Settings.HeightBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, Settings.SelectionBuffer);

}

