
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
#include "Grass/GrassData.h"
#include "Grass/GrassShader.h"
#include "LegitProfiler/GPUProfiler.h"

using namespace std;
using namespace Util;
using namespace GameImports;

namespace TTerrain
{

    int TerrainManager::VerticalQuadLengthLookup[QuadIndexCount0] = {
        0, 6, 6, 12, 18, 6, 12, 12, 12, 6, 18, 18, 12, 18, 24
    };
    
    int TerrainManager::HorizontalQuadLookup[QuadIndexCount0][QuadIndexCount1] = {
        /* Vertices:  0   |    1    |    2   |    3    |    4    |    5   |     6     |     7    */
        /*  0 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /*  1 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 12, 7, 6, 10, 11, 3},
        /*  2 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 5, 11, 13, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /*  3 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /*  4 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /*  5 */{0, 5, 4, 8, 9, 15, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /*  6 */{0, 5, 4, 8, 9, 14, 8, 6, 1, 6, 8, 14, 9, 7, 14, 7, 9, 2, 14, 7, 6, 10, 11, 3},
        /*  7 */{0, 5, 4, 8, 9, 13, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 14, 7, 6, 10, 11, 3},
        /*  8 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /*  9 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 10, 4, 14, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /* 10 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /* 11 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /* 12 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 10, 4, 12, 5, 11, 12, 7, 9, 2, 12, 11, 10, 10, 11, 3},
        /* 13 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
        /* 14 */{0, 5, 4, 4, 5, 12, 8, 6, 1, 6, 8, 13, 9, 7, 14, 7, 9, 2, 15, 11, 10, 10, 11, 3},
    };
    
    int TerrainManager::VerticalQuadLookup[QuadIndexCount0][QuadIndexCount1] = {
        /* 0*/ {- 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 1*/ {6, 10, 11, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 2*/ {5, 9, 11, 9, 11, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 3*/ {5, 9, 14, 14, 12, 5, 13, 15, 10, 10, 6, 13, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 4*/ {5, 9, 14, 14, 12, 5, 13, 15, 10, 10, 6, 13, 14, 15, 11, 11, 7, 14, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 5*/ {4, 8, 9, 9, 5, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 6*/ {4, 8, 9, 9, 5, 4, 6, 10, 11, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 7*/ {4, 8, 9, 9, 5, 4, 6, 10, 11, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 8*/ {4, 8, 13, 13, 12, 4, 14, 15, 11, 11, 7, 14, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /* 9*/ {4, 8, 10, 8, 10, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /*10*/ {4, 8, 13, 13, 12, 4, 14, 15, 11, 11, 7, 14, 13, 15, 10, 10, 6, 13, - 1, - 1, - 1, - 1, - 1, - 1},
        /*11*/ {4, 8, 13, 13, 12, 4, 14, 15, 11, 11, 7, 14, 5, 9, 14, 14, 12, 5, - 1, - 1, - 1, - 1, - 1, - 1},
        /*12*/ {5, 9, 11, 9, 11, 7, 4, 8, 10, 8, 10, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1},
        /*13*/ {5, 9, 14, 14, 12, 5, 13, 15, 10, 10, 6, 13, 4, 8, 13, 13, 12, 4, - 1, - 1, - 1, - 1, - 1, - 1},
        /*14*/ {5, 9, 14, 14, 12, 5, 13, 15, 10, 10, 6, 13, 4, 8, 13, 13, 12, 4, 14, 15, 11, 11, 7, 14},
    };

    
    void TerrainManager::Init()
    {
        RendererPtr = Game::Instance->RendererPtr;
        CamPtr = RendererPtr->GetCamera();
        LightPtr = RendererPtr->GetLight();
        TerrainShader = make_shared<class TerrainShader>();
        GrassShader = make_shared<class GrassShader>();
        GeometryProvider = make_shared<EckyCS::SpriteGeometryProvider>();
        
        CreateCompute();
        CreateTerrainAt(glm::vec3(0.0f));
    }


    void TerrainManager::Render(RenderPassType Type)
    {
        GPU_PROFILE(Game::GetGpuFrame(), "Terrain::Render", legit::Colors::alizarin);
        DispatchCompute();
        RenderTriangles(Type);
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

        
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, ComputeProgramPaint, -1, "DispatchPaint");
        for (auto& Data : TerrainDatas){
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
        glPopDebugGroup();
    }

    void TerrainManager::HandleSelecting() const
    {
        if (bIsSelecting)
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, ComputeProgramSelect, -1, "DispatchSelect");
            glUseProgram(ComputeProgramSelect);
            UpdateComputeVars(ComputeProgramSelect);
            ShaderHelper::SetUniform3fv("_BrushPos", CamPtr->GetMouseWorldPos(), ComputeProgramSelect);
            ShaderHelper::SetUniform3fv("_StartWorldPos", SelectStartWorldPos, ComputeProgramSelect);
            for (const auto& Data : TerrainDatas)
            {
                Data.UpdateComputeVars(ComputeProgramSelect);
                Data.DispatchSelect((GLuint)TerrainSelectionMode::Additive);
            }
            glPopDebugGroup();
        }
        
        // resets the selection once we let go of shift
        if (!bIsSelecting && bWasPressingSelect)
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, ComputeProgramSelect, -1, "DispatchDeSelect");
            glUseProgram(ComputeProgramSelect);
            UpdateComputeVars(ComputeProgramSelect);
            ShaderHelper::SetUniform3fv("_StartWorldPos", SelectStartWorldPos, ComputeProgramSelect);
            for (const auto& Data : TerrainDatas)
            {
                Data.UpdateComputeVars(ComputeProgramSelect);
                Data.DispatchSelect((GLuint)TerrainSelectionMode::Clear);
            }
            glPopDebugGroup();
        }
    }

    void TerrainManager::HandleResetting()
    {
        if (!bIsResetting)
            return;

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, ComputeProgramMesh, -1, "DispatchReset");
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        for (auto& Data : TerrainDatas)
        {
            Data.UpdateComputeVars(ComputeProgramMesh);
            Data.DispatchResetHeight();
            Data.DispatchGenerate();
        }
        glPopDebugGroup();
    }

    void TerrainManager::RenderTriangles(RenderPassType Type)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, TerrainShader->Program, -1, "RenderTerrain");
        TerrainShader->Use(Type);
        auto Settings = GetStandardSettings();
        for (auto& TData : TerrainDatas)
        {
            TData.ApplyToSettings(Settings);
            TerrainShader->UpdateVars(Settings);
            TData.RenderTriangles(Type);
        }
        glPopDebugGroup();
    }
    
    void TerrainManager::OnDrawGizmos(const shared_ptr<Gizmos>& Gizmos) 
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
            ImGui::Checkbox("Grass", &bRenderGrass);
            ImGui::Checkbox("WireFrame", &bShowWireframe);
        
            ImGui::Spacing();
            if (ImGui::Button("Save")) SaveData();
            if (ImGui::Button("Load")) LoadData();
        }
        ImGui::End();
    }

    void TerrainManager::CleanUp() const
    {
        TerrainShader->CleanUp();
        GrassShader->CleanUp();
        for (auto& Data : TerrainDatas)
        {
            Data.CleanUp();
        }
        glDeleteProgram(ComputeProgramMesh);
        glDeleteProgram(ComputeProgramPaint);
        glDeleteProgram(ComputeProgramSelect);
        glDeleteProgram(ComputeProgramGrass);

        glDeleteBuffers(1, &VerticalQuadBuffer);
        glDeleteBuffers(1, &VerticalQuadLengthBuffer);
        glDeleteBuffers(1, &HorizontalQuadBuffer);
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
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        auto& TmpTerrain = TerrainDatas.emplace_back(WorldPos, shared_from_this());
        TmpTerrain.DispatchGenerate();
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
        ComputeProgramMesh = ShaderHelper::CreateComputeProgram({ComputeShaderMesh});
        ComputeProgramPaint = ShaderHelper::CreateComputeProgram({ComputeShaderPaint});
        ComputeProgramSelect = ShaderHelper::CreateComputeProgram({ComputeShaderSelect});
        ComputeProgramGrass = ShaderHelper::CreateComputeProgram({ComputeShaderGrass});
        glObjectLabel(GL_PROGRAM, ComputeProgramMesh, -1, "ComputeProgramMesh");
        glObjectLabel(GL_PROGRAM, ComputeProgramPaint, -1, "ComputeProgramPaint");
        glObjectLabel(GL_PROGRAM, ComputeProgramSelect, -1, "ComputeProgramSelect");
        glObjectLabel(GL_PROGRAM, ComputeProgramGrass, -1, "ComputeProgramGrass");

        glGenBuffers(1, &VerticalQuadBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VerticalQuadBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * QuadIndexCount0 * QuadIndexCount1, VerticalQuadLookup, GL_STATIC_DRAW);
        
        glGenBuffers(1, &VerticalQuadLengthBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VerticalQuadLengthBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * QuadIndexCount0, VerticalQuadLengthLookup, GL_STATIC_DRAW);
        
        glGenBuffers(1, &HorizontalQuadBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, HorizontalQuadBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * QuadIndexCount0 * QuadIndexCount1, HorizontalQuadLookup, GL_STATIC_DRAW);
    }

    void TerrainManager::UpdateComputeVars(GLuint Program) const
    {
        int Dir = GetBrushDirection();
        ShaderHelper::SetUniform2iv("_TexSize", glm::ivec2(TerrainData::TexSize), Program);
        ShaderHelper::SetUniform1ui("_BrushSize", BrushSize, Program);
        ShaderHelper::SetUniform1i("_BrushStrength", BrushStrength * Dir, Program);
        ShaderHelper::SetUniform1ui("_HasSelection", bIsSelecting, Program);
        ShaderHelper::SetUniform1i("VertexLookupLength0", QuadIndexCount0, Program);
        ShaderHelper::SetUniform1i("VertexLookupLength1", QuadIndexCount1, Program);
    }

    GLsizei TerrainManager::GetTotalAppendCount() const
    {
        GLsizei TotalAppendCount = 0;
        for (auto& Data : TerrainDatas)
        {
            TotalAppendCount += Data.AppendCount;
        }
        return TotalAppendCount;
    }

    TerrainShaderSettings TerrainManager::GetStandardSettings() const
    {
        // todo: this gets called / created a lot, streamline!
        static TerrainShaderSettings Settings;
        Settings.RenderPassType = RendererPtr->GetCurrentRenderPassType();
        Settings.BrushPos = CamPtr->GetMouseWorldPos();
        Settings.BrushSize = BrushSize;
        Settings.TexSize = glm::ivec2(TerrainData::TexSize);
        Settings.DirtColor = DirtColor;
        Settings.GrassColor = GrassColor;
        Settings.GrassScale = GrassScale;
        Settings.GrassQuantize = GrassQuantize;
        Settings.bShowWireFrame = bShowWireframe;
        Settings.Camera = CamPtr;
        Settings.Light = LightPtr;
        auto SPass = RendererPtr->GetRenderPass<ShadowPass>();
        Settings.ShadowMap = SPass ? SPass->ColorTex : 0;
        
        // will be filled by the different chunks
        Settings.GlobalWorldPos = glm::vec3(0);
        Settings.VertexBuffer = 0;
        Settings.NormalBuffer = 0;
        Settings.HeightBuffer = 0;
        Settings.SelectionBuffer = 0;
        
        return Settings;
    }

    void TerrainManager::SaveData() const
    {
        size_t DataCount = TerrainDatas.size();
        size_t BufferSize = TerrainData::GetHeightBufferByteSize();

        // we can calculate all sizes from the count
        std::ofstream file("output.bin", std::ios::binary);
        file.write(reinterpret_cast<char*>(&DataCount), sizeof(size_t));
        
        // write WorldPos and Height data for each terrain chunk
        for (auto& TData : TerrainDatas)
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
        TerrainDatas.clear();

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
            auto& TerrainData = TerrainDatas.emplace_back(WorldPos, shared_from_this());
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, TerrainData.HeightBuffer);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, DataPerChunk, Data.data());
        }
        File.close();
        
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        for (auto& TData : TerrainDatas)
        {
            TData.DispatchGenerate();
        }
    }
}