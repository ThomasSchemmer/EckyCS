#define WIN32_LEAN_AND_MEAN

#include <glew/include/GL/glew.h>
#include <GLFW/include/GLFW/glfw3.h>
#include <imgui/imgui.h>
#include "TerrainManager.h"

#include "../Renderer/Camera.h"
#include "../Util/ShaderHelper.h"

#include "TerrainShader.h"

using namespace std;
using namespace Util;

namespace TTerrain
{
    TerrainManager::TerrainManager(const shared_ptr<Camera>& InCamPtr)
    {
        CamPtr = InCamPtr;
        Shader = make_shared<TerrainShader>(Width, Height);
        CreateCompute();
        CreateMesh();
    }

    void TerrainManager::Render()
    {        
        glUseProgram(ComputeProgram);
        UpdateComputeVars();
    
        if (CamPtr->GetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            DispatchGenerate();
        }
        //if (CamPtr->GetKey(GLFW_KEY_O) == GLFW_PRESS)
        {
            DispatchBrush();
        }
    
        Shader->Use();
        Shader->UpdateVars(CamPtr, GetStandardSettings());
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, AppendCount);
    }


    void TerrainManager::DispatchBrush() const
    {
        ShaderHelper::SetUniform3fv("_BrushPos", CamPtr->GetMouseWorldPos(), ComputeProgram);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SelectionBuffer);
        Dispatch(TerrainComputeMode::ApplySelection);
    }


    void TerrainManager::DispatchGenerate() 
    {
        ShaderHelper::ResetBufferCounter(CountBuffer);
    
        glBindImageTexture(0, ResultTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, NormalBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CountBuffer);
        
        // calculate how many vertices we will have
        Dispatch(TerrainComputeMode::CountTriangles);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);

        // allocate the exact amount for the buffers 
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount, nullptr, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, NormalBuffer); // needs only one normal per triangle
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount / 3, nullptr, GL_DYNAMIC_COPY);

        ShaderHelper::ResetBufferCounter(CountBuffer);
    
        // actually compute the vertices & normals
        Dispatch(TerrainComputeMode::GenerateTriangles);
        
        // The count is now the actual amount of triangles to render
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);
    
        //clear binding to make the tex displayable in UI
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    }

    void TerrainManager::OnDrawGizmos() const
    {
        auto Pos = CamPtr->GetMouseWorldPos();
        ImGui::Begin("OpenGL Texture Text");
        ImGui::Text("Pos: %.2f|%.2f|%.2f", Pos.x, Pos.y, Pos.z);
        ImGui::Text("Appnd: %i", AppendCount);
        ImGui::Image((ImTextureID)(intptr_t)ResultTex, ImVec2(Width, Height));
        ImGui::End();
    }

    void TerrainManager::CreateMesh()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Will be filled by compute
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, NormalBuffer);
    }

    void TerrainManager::CreateCompute()
    {
        string ComputeCode = ShaderHelper::LoadShader(ComputePath);
        unsigned int Compute = ShaderHelper::CompileShader(ComputeCode, GL_COMPUTE_SHADER);

        vector IDs = {Compute};
        ComputeProgram = ShaderHelper::CreateProgram(IDs);
        glDeleteShader(Compute);

        glGenTextures(1, &ResultTex);
        glBindTexture(GL_TEXTURE_2D, ResultTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Width, Height, 0,GL_RGBA, GL_FLOAT, nullptr);
        // have to clamp to avoid invalid meshes at the edges
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // we only need to read the required size, then allocate the actual buffers later
        glGenBuffers(1, &CountBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CountBuffer);
    
        glGenBuffers(1, &NormalBuffer);
        glGenBuffers(1, &VertexBuffer);

        glGenBuffers(1, &SelectionBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SelectionBuffer); 
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * Width * Height, nullptr, GL_DYNAMIC_COPY);

        glUseProgram(ComputeProgram);
        UpdateComputeVars();
        Dispatch(TerrainComputeMode::GenerateTex);
    }


    void TerrainManager::UpdateComputeVars() const
    {
        ShaderHelper::SetUniform3fv("_WorldPos", GlobalWorldPos, ComputeProgram);
        ShaderHelper::SetUniform2iv("_TexSize", TexSize, ComputeProgram);
        ShaderHelper::SetUniform3iv("_WorldSize", WorldSize, ComputeProgram);
    }

    void TerrainManager::Dispatch(TerrainComputeMode Mode) const
    {
        ShaderHelper::SetUniform1ui("_Mode", static_cast<GLuint>(Mode), ComputeProgram);
        glDispatchCompute(Width / 32, Height / 32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    TerrainShaderSettings TerrainManager::GetStandardSettings() const
    {
        TerrainShaderSettings Settings;
        Settings.GlobalWorldPos = GlobalWorldPos;
        Settings.ResultTex = ResultTex;
        Settings.VertexBuffer = VertexBuffer;
        Settings.NormalBuffer = NormalBuffer;
        Settings.SelectionBuffer = SelectionBuffer;
        Settings.BrushPos = CamPtr->GetMouseWorldPos();
        Settings.TexSize = glm::ivec2(Width, Height);
        return Settings;
    }


    TerrainManager::~TerrainManager()
    {
        //todo: here and shader: kill program if valid
    }
}