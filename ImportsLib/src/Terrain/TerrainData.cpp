#include "TerrainData.h"
#include <windows.h>

#include "TerrainManager.h"
#include "TerrainShader.h"
#include "../Util/ShaderHelper.h"
//TODO: integratee the updated mesh.comp shader to make height calculation easy through lookup
using namespace Util;
namespace TTerrain
{
    unsigned int TerrainData::TexSize = 32;
    
    void TerrainData::RenderTriangles(RenderPassType Type) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, NormalBuffer);
        glDrawArrays(GL_TRIANGLES, 0, AppendCount);
        if (Manager->bRenderGrass)
        {
            Grass.Render(Type);
        }
    }

    TerrainData::TerrainData(glm::vec3 WorldPos, const std::shared_ptr<TerrainManager>& Manager) :
        GlobalWorldPos(WorldPos), ComputeProgramMesh(Manager->ComputeProgramMesh), ComputeProgramPaint(Manager->ComputeProgramPaint), ComputeProgramSelect(Manager->ComputeProgramSelect),
        VerticalQuadBuffer(Manager->VerticalQuadBuffer), VerticalQuadLengthBuffer(Manager->VerticalQuadLengthBuffer), HorizontalQuadBuffer(Manager->HorizontalQuadBuffer),
        Grass(Manager), Manager(Manager)
    {
        CreateCompute();
        // actual DispatchGenerate() is called from the manager, after settings uniforms!
    }

    void TerrainData::DispatchSelect(GLuint Mode) const
    {
        glUseProgram(ComputeProgramSelect);
        UpdateComputeVars(ComputeProgramSelect);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, HeightBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SelectionBuffer);
        Dispatch(Mode, ComputeProgramSelect);
    }

    void TerrainData::DispatchPaint() const
    {
        glUseProgram(ComputeProgramPaint);
        UpdateComputeVars(ComputeProgramPaint);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SelectionBuffer);
        Dispatch(0, ComputeProgramPaint);
    }

    void TerrainData::DispatchResetHeight() const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, HeightBuffer);
        Dispatch((GLuint)TerrainComputeMode::ResetHeight, ComputeProgramMesh);
    }

    void TerrainData::DispatchGenerate()
    {
        glUseProgram(ComputeProgramMesh);
        UpdateComputeVars(ComputeProgramMesh);
        ShaderHelper::ResetBufferCounter(CountBuffer);
        
        // calculate how many vertices we will have per triangle
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, HeightBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, VertexOffsetsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, VerticalQuadBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, VerticalQuadLengthBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, HorizontalQuadBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, CountBuffer);
        Dispatch((GLuint)TerrainComputeMode::CountTriangles, ComputeProgramMesh);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        // now count them all together
        Dispatch((GLuint)TerrainComputeMode::PrefixSumOffsets, ComputeProgramMesh);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);
                
        // delete old and allocate the exact amount for the buffers
        // can't overwrite/change with BufferStorage!
        if (VertexBuffer != 0) glDeleteBuffers(1, &VertexBuffer);
        if (NormalBuffer != 0) glDeleteBuffers(1, &NormalBuffer);
        glGenBuffers(1, &VertexBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
        glGenBuffers(1, &NormalBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, NormalBuffer); // needs only one normal per triangle
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * AppendCount / 3, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, NormalBuffer);

        ShaderHelper::ResetBufferCounter(CountBuffer);
        
        // actually compute the vertices & normals
        Dispatch((GLuint)TerrainComputeMode::GenerateTriangles, ComputeProgramMesh);
        
        // now we can trigger the grass creation
        Grass.DispatchGenerate(*this);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        size_t ByteCount = sizeof(glm::vec3) * AppendCount;
        void* ptr = glMapBufferRange(
            GL_SHADER_STORAGE_BUFFER, // target
            0,                        // offset (start of range to map)
            ByteCount,        // length (size of range to map)
            GL_MAP_READ_BIT           // access flags
        );
        std::vector<glm::vec3> data_from_gpu(AppendCount);
        memcpy(data_from_gpu.data(), ptr, ByteCount);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }

    void TerrainData::CleanUp() const
    {
        Grass.CleanUp();
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteBuffers(1, &NormalBuffer);
        glDeleteBuffers(1, &HeightBuffer);
        glDeleteBuffers(1, &SelectionBuffer);
        glDeleteBuffers(1, &VertexOffsetsBuffer);
        glDeleteBuffers(1, &CountBuffer);
    }

    void TerrainData::Dispatch(GLuint Mode, GLuint Target)
    {
        ShaderHelper::SetUniform1ui("_Mode", Mode, Target);
        glDispatchCompute(TexSize / 2, TexSize / 2, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void TerrainData::CreateCompute()
    {    
        // both Height and Selection are configurable fixed size
        glGenBuffers(1, &HeightBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, HeightBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr, GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);
        glGenBuffers(1, &SelectionBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SelectionBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr, GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);

        glGenBuffers(1, &VertexOffsetsBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexOffsetsBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr, GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);

        glGenBuffers(1, &CountBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);

        DispatchResetHeight();
    }

    void TerrainData::ApplyToSettings(TerrainShaderSettings& Settings) const
    {
        Settings.GlobalWorldPos = GlobalWorldPos;
        Settings.VertexBuffer = VertexBuffer;
        Settings.NormalBuffer = NormalBuffer;
        Settings.HeightBuffer = HeightBuffer;
        Settings.SelectionBuffer = SelectionBuffer;
    }

    void TerrainData::UpdateComputeVars(GLuint Program) const
    {
        ShaderHelper::SetUniform3fv("_WorldPos", GlobalWorldPos, Program);
        ShaderHelper::SetUniform3iv("_WorldSize", WorldSize, Program);
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
