#include "GrassShader.h"

#include <string>

#include "../TerrainShader.h"
#include "../../Renderer/Camera.h"
#include "../../Renderer/Light.h"
#include "../../Util/ShaderHelper.h"

using namespace std;
using namespace Util;

TTerrain::GrassShader::GrassShader()
{
    
    string VertexCode = ShaderHelper::LoadShaderFromResource(GrassVertexShader);
    string FragmentCode = ShaderHelper::LoadShaderFromResource(GrassFragmentShader);
    unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);
    
    vector IDs = {Vertex, Fragment};
    GrassProgram = ShaderHelper::CreateProgram(IDs);
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    
    Transform = glm::mat4(1.0f);

    GrassTex = ShaderHelper::CreateTexture(GrassTexLocation, GL_RGBA);
    glCreateVertexArrays(1, &VAO);
}

void TTerrain::GrassShader::Use() const
{
    glUseProgram(GrassProgram);
    glBindVertexArray(VAO);
}

void TTerrain::GrassShader::UpdateVars(const GrassShaderSettings& Settings, const TerrainShaderSettings& TerrainSettings) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Settings.VertexBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Settings.PositionBuffer);
    
    ShaderHelper::SetUniformM4("Projection", Settings.Camera->Projection, GrassProgram);
    ShaderHelper::SetUniformM4("View", Settings.Camera->View, GrassProgram);
    ShaderHelper::SetUniformM4("Transform", Transform, GrassProgram);
    ShaderHelper::SetUniform3fv("Right", Settings.Camera->GetRight(), GrassProgram);
    ShaderHelper::SetUniform3fv("Up", Settings.Camera->GetUp(), GrassProgram);
    ShaderHelper::SetUniformTexture("GrassTex", GrassTex, 0, GrassProgram);

    // pass in terrain data for grass color ..
    ShaderHelper::SetUniform3fv("GrassColor", TerrainSettings.GrassColor, GrassProgram);
    ShaderHelper::SetUniform1f("GrassScale", TerrainSettings.GrassScale, GrassProgram);
    ShaderHelper::SetUniform1f("GrassQuantize", TerrainSettings.GrassQuantize, GrassProgram);

    // .. and for shadow mapping
    ShaderHelper::SetUniform3fv("LightDir", TerrainSettings.Light->GetForward(), GrassProgram);
    ShaderHelper::SetUniform3fv("LightPos", TerrainSettings.Light->Position, GrassProgram);
    ShaderHelper::SetUniform2fv("LightClip", TerrainSettings.Light->ClipPlanes, GrassProgram);
    ShaderHelper::SetUniformM4("LightProjection", TerrainSettings.Light->Projection, GrassProgram);
    ShaderHelper::SetUniformM4("LightView", TerrainSettings.Light->View, GrassProgram);
    ShaderHelper::SetUniformTexture("ShadowMap", TerrainSettings.ShadowMap, 1, GrassProgram);
}

void TTerrain::GrassShader::CleanUp() const
{
    glDeleteProgram(GrassProgram);
}
