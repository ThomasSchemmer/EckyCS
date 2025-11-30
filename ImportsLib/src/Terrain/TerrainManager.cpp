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
    
        Shader->Use();
        Shader->UpdateVars(CamPtr, GetStandardSettings());
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, AppendCount);
    }

    void TerrainManager::Update(float Delta)
    {
        HandleInput();
        if (CamPtr->GetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            DispatchGenerate();
        }
        if (bIsSelecting)
        {
            DispatchBrush();
        }
        bWasPressingRaise = bIsRaising;
    }


    void TerrainManager::DispatchBrush() const
    {
        GLuint Mode = static_cast<GLuint>(bIsSelecting ? TerrainSelectionMode::Additive : TerrainSelectionMode::Clear);
        glUseProgram(ComputeProgramSelect);
        ShaderHelper::SetUniform3fv("_BrushPos", CamPtr->GetMouseWorldPos(), ComputeProgramSelect);
        UpdateComputeVars(ComputeProgramSelect);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SelectionBuffer);
        Dispatch(Mode, ComputeProgramSelect);
    }


    void TerrainManager::DispatchGenerate() 
    {
        glUseProgram(ComputeProgramMesh);
        ShaderHelper::ResetBufferCounter(CountBuffer);
    
        glBindImageTexture(0, ResultTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CountBuffer);
        
        // calculate how many vertices we will have
        Dispatch((GLuint)TerrainComputeMode::CountTriangles, ComputeProgramMesh);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);

        // allocate the exact amount for the buffers 
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, NormalBuffer); // needs only one normal per triangle
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount / 3, nullptr, GL_DYNAMIC_STORAGE_BIT);
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, NormalBuffer);

        ShaderHelper::ResetBufferCounter(CountBuffer);
    
        // actually compute the vertices & normals
        Dispatch((GLuint)TerrainComputeMode::GenerateTriangles, ComputeProgramMesh);
        
        // The count is now the actual amount of triangles to render
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);
    
        //clear binding to make the tex displayable in UI
        glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    }

    void TerrainManager::OnDrawGizmos() 
    {
        auto Pos = CamPtr->GetMouseWorldPos();
        ImGui::Begin("TerrainTool");
        ImGui::Text("Pos: %.2f|%.2f|%.2f", Pos.x, Pos.y, Pos.z);
        ImGui::Text("Appnd: %i", AppendCount);
        ImGui::Checkbox("Raise: ", &bIsRaising);
        ImGui::Checkbox("Select: ", &bIsSelecting);
        ImGui::Image((ImTextureID)(intptr_t)ResultTex, ImVec2(Width, Height));
        ImGui::End();
    }

    void TerrainManager::HandleToggle(bool* bIsDoing, bool* bWasDoing, GLint Key) const
    {
        bool bIsPressing = CamPtr->GetKey(Key) == GLFW_PRESS;
        *bIsDoing = !*bWasDoing && bIsPressing ? !*bIsDoing : *bIsDoing;
        *bWasDoing = bIsPressing;
    }
    
    void TerrainManager::HandleToggleMouse(bool* bIsDoing, bool* bWasDoing, GLint Key) const
    {
        bool bIsPressing = CamPtr->GetMouse(Key) == GLFW_PRESS;
        *bIsDoing = !*bWasDoing && bIsPressing ? !*bIsDoing : *bIsDoing;
        *bWasDoing = bIsPressing;
    }

    void TerrainManager::HandleInput()
    {
        HandleToggle(&bIsSelecting, &bWasPressingSelect, GLFW_KEY_LEFT_SHIFT);
        bIsRaising = CamPtr->GetMouse(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
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
        ComputeProgramMesh = ShaderHelper::CreateProgram({ComputeShaderMesh});
        ComputeProgramSelect = ShaderHelper::CreateProgram({ComputeShaderSelect});

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
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * Width * Height, nullptr, GL_DYNAMIC_STORAGE_BIT);

        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        Dispatch((GLuint)TerrainComputeMode::GenerateTex, ComputeProgramMesh);
    }


    void TerrainManager::UpdateComputeVars(GLuint Program) const
    {
        ShaderHelper::SetUniform3fv("_WorldPos", GlobalWorldPos, Program);
        ShaderHelper::SetUniform2iv("_TexSize", TexSize, Program);
        ShaderHelper::SetUniform3iv("_WorldSize", WorldSize, Program);
    }

    void TerrainManager::Dispatch(GLuint Mode, GLuint Target) const
    {
        ShaderHelper::SetUniform1ui("_Mode", Mode, Target);
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