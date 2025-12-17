#pragma once
#include <glm/vec3.hpp>
#include <GL/glew.h>
#include <gl\gl.h>

#include "Grass/GrassData.h"


enum class RenderPassType : uint8_t;

namespace TTerrain
{
    
    enum class TerrainComputeMode : uint8_t
    {
        CountTriangles = 0,
        GenerateTriangles = 1,
        ResetHeight = 2,
    };

    enum class TerrainSelectionMode : uint8_t
    {
        Clear = 0,
        Additive = 1,
    };

    class TerrainShaderSettings;
    /**
     * Holds all relevant data for a single terrain chunk
     * This includes buffer ids for opengl, transform etc
     */
    class TerrainData
    {
    public:
        /** How many triangles are calculated to be necessary */
        GLsizei AppendCount = 0;

        glm::vec3 GlobalWorldPos = glm::vec3(0);
        glm::vec3 WorldSize = glm::vec3(100, 10, 100);

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
        GLuint SelectionBuffer;
        GLuint CountBuffer;
        static unsigned int TexSize;

        GrassData Grass;

        TerrainData(glm::vec3 WorldPos, const std::shared_ptr<TerrainManager>& Manager);
        ~TerrainData() = default;
        void DispatchSelect(GLuint Mode) const;
        void DispatchPaint() const;
        void DispatchResetHeight() const;
        void DispatchGenerate();
        void CleanUp() const;
        static void Dispatch(GLuint Mode, GLuint Target);
        
        void RenderTriangles(RenderPassType Type) const;
        
        void CreateCompute();
        void ApplyToSettings(TerrainShaderSettings& Settings) const;
        void UpdateComputeVars(GLuint Program) const;
        
        static unsigned int GetHeightBufferSize();
        static unsigned int GetHeightBufferByteSize();
    };
}
