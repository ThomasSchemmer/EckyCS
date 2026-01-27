#include "GrassData.h"

#include "GrassShader.h"
#include "../TerrainData.h"
#include "../TerrainHelper.h"
#include "../TerrainManager.h"
#include "../../GameService/Game.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/Passes/RenderPass.h"
#include "../../Util/ShaderHelper.h"
#include "../TerrainShader.h"
#include "imgui/imgui.h"

namespace EckyCS
{
    class RenderData;
}

namespace TTerrain
{
    using namespace Util;
    using namespace EckyCS;
    
    GrassData::GrassData(const shared_ptr<TerrainManager>& InManager)
    {
        GrassCompute = InManager->ComputeProgramGrass;
        GrassShader = InManager->GrassShader;
        Manager = InManager;
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VertexBuffer);

        // sprite is going to stay the same, but offset will have to be recreated
        glBindVertexArray(VAO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER, InManager->GeometryProvider->GetVertexByteCount(),
            InManager->GeometryProvider->GetVertexArray(), GL_STATIC_DRAW
        );
        
        glGenBuffers(1, &CountBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_STATIC_COPY);
    }

    void GrassData::DispatchGenerate(const TerrainData& Data)
    {
        glUseProgram(GrassCompute);
        ShaderHelper::ResetBufferCounter(CountBuffer);

        ShaderHelper::SetUniform1ui("_Mode", 0, GrassCompute);
        ShaderHelper::SetUniform2iv("_TexSize", glm::vec2(TerrainData::TexSize), GrassCompute);
        ShaderHelper::SetUniform2iv("_TargetCount", TargetCount, GrassCompute);
        ShaderHelper::SetUniform3iv("_WorldSize", TerrainData::WorldSize, GrassCompute);
        ShaderHelper::SetUniform3fv("_GlobalWorldPos", Data.GlobalWorldPos, GrassCompute);
        ShaderHelper::SetUniform1ui("_GroupCount", GroupCount, GrassCompute);
    
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CountBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Data.VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Data.VertexOffsetsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, Data.HeightBuffer);
        glDispatchCompute(GroupCount, GroupCount, 1);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);

        if (PositionBuffer != 0) glDeleteBuffers(1, &PositionBuffer);
        glGenBuffers(1, &PositionBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, PositionBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount, nullptr, GL_STATIC_DRAW);

        ShaderHelper::ResetBufferCounter(CountBuffer);
        ShaderHelper::SetUniform1ui("_Mode", 1, GrassCompute);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, PositionBuffer);
        glDispatchCompute(GroupCount, GroupCount, 1);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);
    }

    void GrassData::CleanUp() const
    {
        glDeleteBuffers(1, &PositionBuffer);
        glDeleteBuffers(1, &CountBuffer);
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteVertexArrays(1, &VAO);
    }
    
    void GrassData::Render(RenderPassType Type) const
    {
        if (Type != RenderPassType::BasePass)
            return;

        GrassShader->Use();
        GrassShader->UpdateVars( GetStandardSettings(), Manager->GetStandardSettings());
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, AppendCount);
    }

    GrassShaderSettings& GrassData::GetStandardSettings() const
    {
        static GrassShaderSettings Settings;
        Settings.VertexBuffer = VertexBuffer;
        Settings.PositionBuffer = PositionBuffer;
        Settings.Camera = GameImports::Game::Instance->RendererPtr->GetCamera();
        return Settings;
    }
}
