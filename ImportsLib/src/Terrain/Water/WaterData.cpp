#include "WaterData.h"

#include "../TerrainData.h"
#include "../TerrainManager.h"
#include "GL/glew.h"
#include "../../Util/ShaderHelper.h"

using namespace Util;

namespace TTerrain
{
    WaterData::WaterData()
    {
        CreateCompute();
    }

    WaterData::~WaterData()
    {
        glDeleteBuffers(1, &CountBuffer);
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteBuffers(1, &VertexOffsetBuffer);
    }

    void WaterData::DispatchGenerate(TerrainData& Data)
    {
        glUseProgram(Data.ComputeProgramMesh);
        ShaderHelper::ResetBufferCounter(CountBuffer);
        
        ShaderHelper::SetUniform1ui("TargetHeightMask", LAYOUT_WATER, Data.ComputeProgramMesh);
        
        // calculate how many vertices we will have per triangle
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHeights, Data.HeightBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertexOffsets, VertexOffsetBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVerticalQuads, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVerticalQuadLengths, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOHorizontalQuads, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOCount, CountBuffer);
        Dispatch((GLuint)TerrainComputeMode::CountTriangles, Data.ComputeProgramMesh);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        // now count them all together
        Dispatch((GLuint)TerrainComputeMode::PrefixSumOffsets, Data.ComputeProgramMesh);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        AppendCount = ShaderHelper::ReadBufferCount(CountBuffer);

        CreateTempCompute();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertices, VertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBONormals, 0);

        ShaderHelper::ResetBufferCounter(CountBuffer);
        
        // actually compute the vertices & normals
        Dispatch((GLuint)TerrainComputeMode::GenerateTriangles, Data.ComputeProgramMesh);
    }

    void WaterData::CreateTempCompute()
    {
        // delete old and allocate the exact amount for the buffers
        // can't overwrite/change with BufferStorage!
        if (VertexBuffer != 0)
        {
            glUnmapBuffer(VertexBuffer);
            glDeleteBuffers(1, &VertexBuffer);
        }

        const int VertexSize = sizeof(float) * 4 * AppendCount; 

        glGenBuffers(1, &VertexBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexBuffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, VertexSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    void WaterData::CreateCompute()
    {
        glGenBuffers(1, &VertexOffsetBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, VertexOffsetBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TerrainData::GetHeightBufferByteSize(), nullptr, GL_DYNAMIC_STORAGE_BIT);

        glGenBuffers(1, &CountBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CountBuffer); 
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    void WaterData::Dispatch(GLuint Mode, GLuint Target)
    {
        ShaderHelper::SetUniform1ui("_Mode", Mode, Target);
        glDispatchCompute(TerrainData::TexSize / 2, TerrainData::TexSize / 2, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void WaterData::Render(RenderPassType Type) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBOVertices, VertexBuffer);
        glDrawArrays(GL_TRIANGLES, 0, AppendCount);
    }
}
