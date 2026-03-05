#include "WaterShader.h"
#include "../../Util/ShaderHelper.h"
#include "../TerrainManager.h"
#include "../../Renderer/Camera.h"
#include "../../Renderer/Light.h"

using namespace std;
using namespace Util;

namespace TTerrain
{
    WaterShader::WaterShader()
    {
        string VertexCode = ShaderHelper::LoadShaderFromResource(VertexShader);
        string FragmentCode = ShaderHelper::LoadShaderFromResource(FragmentShader);

        unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
        unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

        vector IDs = {Vertex, Fragment};
        Program = ShaderHelper::CreateProgram(IDs);
        glObjectLabel(GL_PROGRAM, Program, -1, "WaterShaderProgram");
    
        glDeleteShader(Vertex);
        glDeleteShader(Fragment);
    
        Transform = glm::mat4(1.0f);
    }

    void WaterShader::Use(RenderPassType Type)
    {
        glUseProgram(Program);
    }

    
    void WaterShader::UpdateVars(const WaterShaderSettings& Settings) const
    {
        // where should this be placed in world space?
        ShaderHelper::SetUniform2iv("TexSize", Settings.TexSize, Program);
        ShaderHelper::SetUniform3fv("GlobalWorldPos", Settings.GlobalWorldPos, Program);
        ShaderHelper::SetUniformM4("Transform", Transform, Program);

        // rendering info, useful for shadows etc
        ShaderHelper::SetUniformM4("Projection", Settings.Camera->Projection, Program);
        ShaderHelper::SetUniformM4("View", Settings.Camera->View, Program);
        ShaderHelper::SetUniform3fv("LightDir", Settings.Light->GetForward(), Program);
        ShaderHelper::SetUniform3fv("LightPos", Settings.Light->Position, Program);
        ShaderHelper::SetUniform2fv("LightClip", Settings.Light->ClipPlanes, Program);
        ShaderHelper::SetUniformM4("LightProjection", Settings.Light->Projection, Program);
        ShaderHelper::SetUniformM4("LightView", Settings.Light->View, Program);
        ShaderHelper::SetUniformTexture("DepthTex", Settings.DepthTex, 0, Program);
        
        ShaderHelper::SetUniform2fv("DepthThreshold", Settings.DepthThreshold, Program);
        ShaderHelper::SetUniform2fv("ScreenSize", Settings.ScreenSize, Program);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertices, Settings.VertexBuffer);
    }

    void WaterShader::CleanUp() const
    {
        glDeleteProgram(Program);
    }
}
