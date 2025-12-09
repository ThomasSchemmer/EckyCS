
#include <glew/include/GL/glew.h>
#include <GLFW/include/GLFW/glfw3.h>
#include <imgui/imgui.h>
#include "TerrainManager.h"

#include "TerrainData.h"
#include "../Renderer/Camera.h"
#include "../Util/ShaderHelper.h"
#include "../GameService/Game.h"

#include "TerrainShader.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Passes/ShadowPass.h"

using namespace std;
using namespace Util;
using namespace GameImports;

namespace TTerrain
{
    TerrainManager::TerrainManager()
    {
        RendererPtr = Game::Instance->RendererPtr;
        CamPtr = RendererPtr->GetCamera();
        LightPtr = RendererPtr->GetLight();
        Shader = make_shared<TerrainShader>();
        CreateCompute();
        CreateTerrainAt(glm::vec3(-5, 0, -5));
    }

    void TerrainManager::Render()
    {       
        DispatchCompute();
        RenderTriangles();
        bWasPressingRaise = bIsRaising;
        bWasPressingSelect = bIsSelecting;
    }

    void TerrainManager::Update(float Delta)
    {
        HandleInput();
    }

    void TerrainManager::DispatchCompute()
    {
        if (!bIsEditing)
            return;
        
        HandleResetting();
        HandleSelecting();
        HandlePainting();
    }

    void TerrainManager::HandlePainting()
    { 
        if (bIsRaising || !bWasPressingRaise)
            return;

        for (auto& Data : Datas){
            glUseProgram(ComputeProgramPaint);
            ShaderHelper::SetUniform3fv("_BrushPos", RaiseStartWorldPos, ComputeProgramPaint);
            UpdateComputeVars(ComputeProgramPaint);
            Data.DispatchPaint();

            glUseProgram(ComputeProgramMesh);
            UpdateComputeVars(ComputeProgramMesh);
            Data.DispatchGenerate();

            glUseProgram(ComputeProgramSelect);
            UpdateComputeVars(ComputeProgramSelect);
            Data.DispatchSelect((GLuint)TerrainSelectionMode::Clear);
        }
    }

    void TerrainManager::HandleSelecting() const
    {
        if (bIsSelecting)
        {
            glUseProgram(ComputeProgramSelect);
            UpdateComputeVars(ComputeProgramSelect);
            ShaderHelper::SetUniform3fv("_BrushPos", CamPtr->GetMouseWorldPos(), ComputeProgramSelect);
            ShaderHelper::SetUniform3fv("_StartWorldPos", SelectStartWorldPos, ComputeProgramSelect);
            for (const auto& Data : Datas)
            {
                Data.UpdateComputeVars(ComputeProgramSelect);
                Data.DispatchSelect((GLuint)TerrainSelectionMode::Additive);
            }
        }
        
        // resets the selection once we let go of shift
        if (!bIsSelecting && bWasPressingSelect)
        {
            glUseProgram(ComputeProgramSelect);
            UpdateComputeVars(ComputeProgramSelect);
            ShaderHelper::SetUniform3fv("_StartWorldPos", SelectStartWorldPos, ComputeProgramSelect);
            for (const auto& Data : Datas)
            {
                Data.UpdateComputeVars(ComputeProgramSelect);
                Data.DispatchSelect((GLuint)TerrainSelectionMode::Clear);
            }
        }
    }

    void TerrainManager::HandleResetting()
    {
        if (!bIsResetting)
            return;

        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        for (auto& Data : Datas)
        {
            Data.UpdateComputeVars(ComputeProgramMesh);
            Data.DispatchResetHeight();
            Data.DispatchGenerate();
        }
    }

    void TerrainManager::RenderTriangles() const
    {
        Shader->Use();
        auto Settings = GetStandardSettings();
        for (const auto& TData : Datas)
        {
            TData.ApplyToSettings(Settings);
            Shader->UpdateVars(Settings);
            TData.RenderTriangles();
        }
    }
    
    void TerrainManager::OnDrawGizmos() 
    {
        auto Pos = CamPtr->GetMouseWorldPos();
        ImGui::Begin("TerrainTool");
        ImGui::Checkbox("Editable", &bIsEditing);
        if (bIsEditing)
        {
            ImGui::Text("Pos: %.2f|%.2f|%.2f", Pos.x, Pos.y, Pos.z);
            ImGui::Text("Appnd: %i", GetTotalAppendCount());
            ImGui::Checkbox("Raise: ", &bIsRaising);
            ImGui::Checkbox("Select: ", &bIsSelecting);
            ImGui::Text("Brush Settings:");
            ImGui::SliderInt("Size: ", &BrushSize, 1, 10);
            ImGui::SliderInt("Strength: ", &BrushStrength, 1, 10);
        
            glm::vec2 Diff;
            CamPtr->GetMouseCoords(Diff);
            Diff = BrushStartScreenPos - Diff;
            ImGui::Text("Diff: %.2f|%.2f", Diff.x, Diff.y);
        
            ImGui::Spacing();
            ImGui::SetNextItemWidth(150);
            ImGui::ColorPicker3("Dirt", glm::value_ptr(DirtColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions);
            ImGui::SetNextItemWidth(150);
            ImGui::ColorPicker3("Grass", glm::value_ptr(GrassColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions);
            ImGui::SliderFloat("Scale: ", &GrassScale, 0, 0.25);
            ImGui::SliderFloat("Quantize: ", &GrassQuantize, 2, 10);
        
            ImGui::Spacing();
            if (ImGui::Button("Save")) SaveData();
            if (ImGui::Button("Load")) LoadData();
        }
        ImGui::End();
    }

    void TerrainManager::CleanUp() const
    {
        for (auto& Data : Datas)
        {
            Data.CleanUp();
        }
        glDeleteProgram(ComputeProgramMesh);
        glDeleteProgram(ComputeProgramPaint);
        glDeleteProgram(ComputeProgramSelect);
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

    int TerrainManager::GetBrushDirection() const
    {
        glm::vec2 BrushEnd;
        CamPtr->GetMouseCoords(BrushEnd);
        // y dir is flipped in opengl
        return -static_cast<int>(glm::sign(BrushEnd.y - BrushStartScreenPos.y));
    }

    void TerrainManager::CreateTerrainAt(glm::vec3 WorldPos)
    {
        auto& Tmp = Datas.emplace_back(WorldPos, ComputeProgramMesh, ComputeProgramPaint, ComputeProgramSelect);
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        Tmp.DispatchGenerate();
    }

    void TerrainManager::HandleInput()
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return;
        
        bIsSelecting = CamPtr->GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        bIsRaising = CamPtr->GetMouse(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bIsResetting = CamPtr->GetKey(GLFW_KEY_O) == GLFW_PRESS;
        if (bIsRaising && !bWasPressingRaise)
        {
            CamPtr->GetMouseCoords(BrushStartScreenPos);
            RaiseStartWorldPos = CamPtr->GetMouseWorldPos();
        }
        if (bIsSelecting && !bWasPressingSelect)
        {
            SelectStartWorldPos = CamPtr->GetMouseWorldPos();
        }
    }

    void TerrainManager::CreateCompute()
    {
        ComputeProgramMesh = ShaderHelper::CreateProgram({ComputeShaderMesh});
        ComputeProgramPaint = ShaderHelper::CreateProgram({ComputeShaderPaint});
        ComputeProgramSelect = ShaderHelper::CreateProgram({ComputeShaderSelect});
    }

    void TerrainManager::UpdateComputeVars(GLuint Program) const
    {
        int Dir = GetBrushDirection();
        ShaderHelper::SetUniform2iv("_TexSize", glm::ivec2(TerrainData::TexSize), Program);
        ShaderHelper::SetUniform1ui("_BrushSize", BrushSize, Program);
        ShaderHelper::SetUniform1i("_BrushStrength", BrushStrength * Dir, Program);
        ShaderHelper::SetUniform1ui("_HasSelection", bIsSelecting, Program);
    }

    GLsizei TerrainManager::GetTotalAppendCount() const
    {
        GLsizei TotalAppendCount = 0;
        for (auto& Data : Datas)
        {
            TotalAppendCount += Data.AppendCount;
        }
        return TotalAppendCount;
    }

    TerrainShaderSettings TerrainManager::GetStandardSettings() const
    {
        static TerrainShaderSettings Settings;
        Settings.RenderPassType = RendererPtr->GetCurrentRenderPassType();
        Settings.BrushPos = CamPtr->GetMouseWorldPos();
        Settings.BrushSize = BrushSize;
        Settings.TexSize = glm::ivec2(TerrainData::TexSize);
        Settings.DirtColor = DirtColor;
        Settings.GrassColor = GrassColor;
        Settings.GrassScale = GrassScale;
        Settings.GrassQuantize = GrassQuantize;
        Settings.Camera = CamPtr;
        Settings.Light = LightPtr;
        Settings.ShadowMap = RendererPtr->GetRenderPass<ShadowPass>()->DepthTex;
        
        // will be filled by the different chunks
        Settings.GlobalWorldPos = glm::vec3(0);
        Settings.VertexBuffer = 0;
        Settings.NormalBuffer = 0;
        Settings.HeightBuffer = 0;
        Settings.SelectionBuffer = 0;
        Settings.WorldSize = glm::vec3(0);
        
        return Settings;
    }

    void TerrainManager::SaveData() const
    {
        size_t DataCount = Datas.size();
        size_t BufferSize = TerrainData::GetHeightBufferSize();

        // we can calculate all sizes from the count
        std::ofstream file("output.bin", std::ios::binary);
        file.write(reinterpret_cast<char*>(&DataCount), sizeof(size_t));
        
        // write WorldPos and Height data for each terrain chunk
        for (auto& TData : Datas)
        {
            file.write(reinterpret_cast<const char*>(&TData.GlobalWorldPos.x), sizeof(glm::vec3));
            std::vector<uint32_t> Data(BufferSize / sizeof(uint32_t));
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, TData.HeightBuffer);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, BufferSize, Data.data());
            file.write(reinterpret_cast<char*>(Data.data()), BufferSize);
        }

        file.close();
    }

    void TerrainManager::LoadData()
    {
        Datas.clear();

        // basically the reverse of @SaveData()
        std::ifstream File("output.bin", std::ios::binary);

        File.seekg(0, std::ios::end);
        size_t FileSize = File.tellg();
        File.seekg(0, std::ios::beg);
        size_t ChunkCount;
        File.read(reinterpret_cast<char*>(&ChunkCount), sizeof(size_t));

        FileSize -= sizeof(size_t);
        size_t BytesPerChunk = FileSize / ChunkCount;
        size_t DataPerChunk = BytesPerChunk - sizeof(glm::vec3);

        std::vector<uint32_t> Data;
        Data.resize(DataPerChunk / sizeof(uint32_t));
        for (size_t i = 0; i < ChunkCount; i++)
        {
            glm::vec3 WorldPos;
            File.read(reinterpret_cast<char*>(&WorldPos), sizeof(glm::vec3));
            File.read(reinterpret_cast<char*>(Data.data()), DataPerChunk);
            auto& TerrainData = Datas.emplace_back(WorldPos, ComputeProgramMesh, ComputeProgramPaint, ComputeProgramSelect);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, TerrainData.HeightBuffer);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, DataPerChunk, Data.data());
        }
        File.close();
        
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        for (auto& TData : Datas)
        {
            TData.DispatchGenerate();
        }
    }
}