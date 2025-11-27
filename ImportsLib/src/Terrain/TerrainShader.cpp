#include "TerrainShader.h"
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>
#include "../Renderer/Camera.h"
#include "../Util/ShaderHelper.h"
using namespace Util;

TTerrain::TerrainShader::TerrainShader(int InWidth, int InHeight) :
    Width(InWidth), Height(InHeight)
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

TTerrain::TerrainShader::~TerrainShader()
{
}

void TTerrain::TerrainShader::Use() const
{
    glUseProgram(Program);
}

void TTerrain::TerrainShader::UpdateVars(const shared_ptr<Camera>& Camera, const TerrainShaderSettings& Settings) const
{
    ShaderHelper::SetUniform3fv("GlobalWorldPos", Settings.GlobalWorldPos, Program);
    ShaderHelper::SetUniform1f("BrushSize", 2, Program);
    ShaderHelper::SetUniform3fv("BrushPos", Settings.BrushPos, Program);
    ShaderHelper::SetUniformM4("Transform", Transform, Program);
    ShaderHelper::SetUniformM4("Projection", Camera->Projection, Program);
    ShaderHelper::SetUniformM4("View", Camera->View, Program);

    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Settings.VertexBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Settings.NormalBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Settings.SelectionBuffer);

    ShaderHelper::SetUniformTexture("ResultTex", Settings.ResultTex, 0, Program);
    ShaderHelper::SetUniform2iv("TexSize", Settings.TexSize, Program);
}

