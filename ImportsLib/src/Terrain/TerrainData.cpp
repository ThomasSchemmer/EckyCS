#include "TerrainData.h"
#define NOMINMAX
#include <windows.h>

#include "TerrainManager.h"
#include "TerrainShader.h"
#include "../GameService/Game.h"
#include "../Util/ShaderHelper.h"
#include "LegitProfiler/GPUProfiler.h"

using namespace Util;
namespace TTerrain
{
    unsigned int TerrainData::TexSize = 32;
    glm::vec3 TerrainData::WorldSize = glm::vec3(100, 10, 100);
    glm::vec2 TerrainData::WorldSize2D = glm::vec2(WorldSize.x, WorldSize.z);
    
    void TerrainData::RenderBase(RenderPassType Type) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertices, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBONormals, NormalBuffer);
        glDrawArrays(GL_TRIANGLES, 0, AppendCount);
        if (Manager->bRenderGrass)
        {
            GPU_PROFILE(GameImports::Game::GetGpuFrame(), "Grass", legit::Colors::greenSea);
            Grass.Render(Type);
        }
    }

    void TerrainData::RenderWater(RenderPassType Type) const
    {
        GPU_PROFILE(GameImports::Game::GetGpuFrame(), "Water", legit::Colors::peterRiver);
        Water.Render(Type);
    }

    TerrainData::TerrainData(glm::vec3 WorldPos, const std::shared_ptr<TerrainManager>& Manager) :
        GlobalWorldPos(WorldPos), ComputeProgramMesh(Manager->ComputeProgramMesh), ComputeProgramRaise(Manager->ComputeProgramPaint), ComputeProgramSelect(Manager->ComputeProgramSelect),
        VerticalQuadBuffer(Manager->VerticalQuadBuffer), VerticalQuadLengthBuffer(Manager->VerticalQuadLengthBuffer), HorizontalQuadBuffer(Manager->HorizontalQuadBuffer),
        Grass(Manager), Manager(Manager)
    {
        CreateCompute();
        // actual DispatchGenerate() is called from the manager, after settings uniforms!
    }

    void TerrainData::DispatchSelect(GLuint Mode, GLuint Target)
    {
        glUseProgram(ComputeProgramSelect);
        UpdateComputeVars(ComputeProgramSelect);
        ShaderHelper::SetUniform1ui("_Target", Target, ComputeProgramSelect);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHeights, HeightBuffer);
        Dispatch(Mode, ComputeProgramSelect);
    }

    void TerrainData::DispatchRaise(GLuint Mode)
    {
        glUseProgram(ComputeProgramRaise);
        UpdateComputeVars(ComputeProgramRaise);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHeights, HeightBuffer);
        Dispatch(Mode, ComputeProgramRaise);
    }

    void TerrainData::DispatchResetHeight()
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHeights, HeightBuffer);
        Dispatch((GLuint)TerrainComputeMode::ResetHeight, ComputeProgramMesh);
    }

    void TerrainData::DispatchGenerate()
    {
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        ShaderHelper::ResetBufferCounter(CountBuffer);
        
        // calculate how many vertices we will have per triangle
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHeights, HeightBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertexOffsets, VertexOffsetsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVerticalQuads, VerticalQuadBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVerticalQuadLengths, VerticalQuadLengthBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHorizontalQuads, HorizontalQuadBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOCount, CountBuffer);
        Dispatch((GLuint)TerrainComputeMode::CountTriangles, ComputeProgramMesh);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        // now count them all together
        Dispatch((GLuint)TerrainComputeMode::PrefixSumOffsets, ComputeProgramMesh);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);

        CreateTempCompute();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertices, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBONormals, NormalBuffer);

        ShaderHelper::ResetBufferCounter(CountBuffer);
        
        // actually compute the vertices & normals
        Dispatch((GLuint)TerrainComputeMode::GenerateTriangles, ComputeProgramMesh);
        
        // now we can trigger the grass/water creation
        Grass.DispatchGenerate(*this);
        Water.DispatchGenerate(*this);
    }

    void TerrainData::CreateTempCompute()
    {
        // delete old and allocate the exact amount for the buffers
        // can't overwrite/change with BufferStorage!
        if (VertexBuffer != 0)
        {
            glUnmapBuffer(VertexBuffer);
            glDeleteBuffers(1, &VertexBuffer);
        }
        if (NormalBuffer != 0)
        {
            glDeleteBuffers(1, &NormalBuffer);
        }
        glGenBuffers(1, &VertexBuffer);

        const unsigned int VertexSize = sizeof(float) * 4 * AppendCount; 
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, VertexSize, nullptr,
            GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT);
        MappedVertexPtr = glMapNamedBufferRange(
            VertexBuffer,
            0,
            VertexSize,
            GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        
        glGenBuffers(1, &NormalBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, NormalBuffer); // needs only one normal per triangle
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, VertexSize / 3, nullptr,
            GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);

    }

    void TerrainData::CleanUp() const
    {
        Grass.CleanUp();
        Water.CleanUp();
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteBuffers(1, &NormalBuffer);
        glDeleteBuffers(1, &HeightBuffer);
        glDeleteBuffers(1, &VertexOffsetsBuffer);
        glDeleteBuffers(1, &CountBuffer);
    }

    bool TerrainData::IsDirty() const
    {
        return bIsDirty;
    }

    void TerrainData::Dispatch(GLuint Mode, GLuint Target)
    {
        ShaderHelper::SetUniform1ui("_Mode", Mode, Target);
        glDispatchCompute(TexSize / 2, TexSize / 2, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        bIsDirty = true;
    }

    void TerrainData::CreateCompute()
    {    
        // both Height and Selection are configurable fixed size and will be synced with CPU!
        glGenBuffers(1, &HeightBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, HeightBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr,
            GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        MappedHeightPtr = glMapNamedBufferRange(
            HeightBuffer,
            0,
            GetHeightBufferByteSize(),
            GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );

        glGenBuffers(1, &VertexOffsetsBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexOffsetsBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr,
            GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        MappedVertexOffsetPtr = glMapNamedBufferRange(
            VertexOffsetsBuffer,
            0,
            GetHeightBufferByteSize(),
            GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );

        glGenBuffers(1, &CountBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr,
            GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);

        DispatchResetHeight();
    }

    void TerrainData::ApplyToSettings(TerrainShaderSettings& Settings) const
    {
        Settings.GlobalWorldPos = GlobalWorldPos;
        Settings.VertexBuffer = VertexBuffer;
        Settings.NormalBuffer = NormalBuffer;
        Settings.HeightBuffer = HeightBuffer;
    }

    void TerrainData::ApplyToSettings(WaterShaderSettings& Settings) const
    {
        Settings.GlobalWorldPos = GlobalWorldPos;
        Settings.VertexBuffer = VertexBuffer;
    }

    void TerrainData::UpdateComputeVars(GLuint Program) const
    {
        ShaderHelper::SetUniform3fv("_WorldPos", GlobalWorldPos, Program);
        ShaderHelper::SetUniform3iv("_WorldSize", WorldSize, Program);
        ShaderHelper::SetUniform1ui("TargetHeightMask", TerrainManager::LAYOUT_HEIGHT_MASK, Program);
        ShaderHelper::SetUniform1ui("TargetHeightOffset", TerrainManager::LAYOUT_HEIGHT_OFFSET, Program);
    }

    unsigned int TerrainData::GetHeightBufferSize()
    {
        return TexSize * TexSize;
    }

    unsigned int TerrainData::GetHeightBufferByteSize()
    {
        return sizeof(unsigned int) * GetHeightBufferSize();
    }
}
