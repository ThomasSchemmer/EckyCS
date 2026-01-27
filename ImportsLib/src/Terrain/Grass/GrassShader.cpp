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
    glObjectLabel(GL_PROGRAM, GrassProgram, -1, "GrassShaderProgram");
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    
    Transform = glm::mat4(1.0f);

    vector List = {GrassTexLocation, FlowerTexLocation};
    auto Result = ShaderHelper::CreateTextureArray(List, 256, 256, GL_RGBA);
    FoliageTexArray = Result[0];
    GrassTex = Result[1];
    FlowerTex = Result[2];
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
    glBindTextureUnit(0, FoliageTexArray);
    
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
    glDeleteTextures(1, &FoliageTexArray);
    glDeleteTextures(1, &GrassTex);
    glDeleteTextures(1, &FlowerTex);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(GrassProgram);
    
}
