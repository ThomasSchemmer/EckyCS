#include "TerrainData.h"
#include <windows.h>

#include "TerrainManager.h"
#include "TerrainShader.h"
#include "../Util/ShaderHelper.h"

using namespace Util;
namespace TTerrain
{
    unsigned int TerrainData::TexSize = 32;
    
    void TerrainData::RenderTriangles(RenderPassType Type) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, NormalBuffer);
        glDrawArrays(GL_TRIANGLES, 0, AppendCount);
        Grass.Render(Type);
    }

    TerrainData::TerrainData(glm::vec3 WorldPos, const std::shared_ptr<TerrainManager>& Manager) :
        GlobalWorldPos(WorldPos), ComputeProgramMesh(Manager->ComputeProgramMesh), ComputeProgramPaint(Manager->ComputeProgramPaint), ComputeProgramSelect(Manager->ComputeProgramSelect),
        Grass(Manager)
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
        
        // calculate how many vertices we will have
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CountBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, HeightBuffer);
        Dispatch((GLuint)TerrainComputeMode::CountTriangles, ComputeProgramMesh);
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
        
        // The count is now the actual amount of triangles to render
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);

        // now we can trigger the grass creation
        Grass.DispatchGenerate(*this);
    }

    void TerrainData::CleanUp() const
    {
        Grass.CleanUp();
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteBuffers(1, &NormalBuffer);
        glDeleteBuffers(1, &CountBuffer);
        glDeleteBuffers(1, &HeightBuffer);
        glDeleteBuffers(1, &SelectionBuffer);
    }

    void TerrainData::Dispatch(GLuint Mode, GLuint Target)
    {
        ShaderHelper::SetUniform1ui("_Mode", Mode, Target);
        glDispatchCompute(TexSize / 2, TexSize / 2, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void TerrainData::CreateCompute()
    {
        // we only need to read the required size, then allocate the actual buffers later
        glGenBuffers(1, &CountBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CountBuffer);
    
        // both Height and Selection are configurable fixed size
        glGenBuffers(1, &HeightBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, HeightBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glGenBuffers(1, &SelectionBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SelectionBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, GetHeightBufferByteSize(), nullptr, GL_DYNAMIC_STORAGE_BIT);

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
