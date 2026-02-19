#pragma once
#include <map>
#include <glm/vec3.hpp>
#include <GL/glew.h>
#include <gl\gl.h>

#include "Grass/GrassData.h"
#include "Water/WaterData.h"


enum class RenderPassType : uint8_t;

namespace TTerrain
{
    class WaterShaderSettings;

    class TerrainShaderSettings;
    
    enum class TerrainComputeMode : uint8_t
    {
        CountTriangles = 0,
        GenerateTriangles = 1,
        ResetHeight = 2,
        PrefixSumOffsets = 3
    };

    /** How to overwrite the data slot*/
    enum class TerrainSelectionMode : uint8_t
    {
        Clear = 0,
        Select = 1,
        DeSelect = 2
    };

    /** Which data slot to overwrite */
    enum class TerrainTarget : uint8_t
    {
        Selection = 0,
        Tex0 = 1,
        Tex1 = 2,
        Tex2 = 3,
        Grass = 4,
        Flower = 5
    };

    enum class TerrainPaintMode : uint8_t
    {
        ApplyHeight = 0,
    };

    
    /**
     * Holds all relevant data for a single terrain chunk
     * This includes buffer ids for opengl, transform etc
     * Careful: Each TerrainData has its dependent @WaterData and @GrassData
     * included in its lifecycle!
     */
    class TerrainData
    {
    public:
        /** How many triangles are calculated to be necessary */
        GLsizei AppendCount = 0;

        glm::vec3 GlobalWorldPos = glm::vec3(0);

        /** Owned by the manager! */
        GLuint ComputeProgramMesh;
        GLuint ComputeProgramPaint;
        GLuint ComputeProgramSelect;
        GLuint VerticalQuadBuffer;
        GLuint VerticalQuadLengthBuffer;
        GLuint HorizontalQuadBuffer;
        
        GLuint VertexBuffer;
        GLuint NormalBuffer;
        GLuint HeightBuffer;
        GLuint VertexOffsetsBuffer;
        GLuint CountBuffer;
        static unsigned int TexSize;
        static glm::vec3 WorldSize;
        static glm::vec2 WorldSize2D;

        GrassData Grass;
        WaterData Water;

        TerrainData(glm::vec3 WorldPos, const std::shared_ptr<TerrainManager>& Manager);
        ~TerrainData() = default;
        void DispatchSelect(GLuint Mode, GLuint Target);
        void DispatchRaise(GLuint Mode);
        void DispatchResetHeight();
        void DispatchGenerate();
        void Dispatch(GLuint Mode, GLuint Target);
        void CleanUp() const;
        bool IsDirty() const;
        
        void RenderBase(RenderPassType Type) const;
        void RenderWater(RenderPassType Type) const;
        
        void CreateTempCompute();
        void CreateCompute();
        void ApplyToSettings(TerrainShaderSettings& Settings) const;
        void ApplyToSettings(WaterShaderSettings& Settings) const;
        void UpdateComputeVars(GLuint Program) const;

        std::shared_ptr<TerrainManager> Manager;
        
        static unsigned int GetHeightBufferSize();
        static unsigned int GetHeightBufferByteSize();

        void* MappedHeightPtr = nullptr;
        void* MappedVertexPtr = nullptr;
        void* MappedVertexOffsetPtr = nullptr;
        bool bIsDirty = false;
    };
}
