#pragma once
#include <glm/vec3.hpp>
#include <GL/glew.h>
#include <gl\gl.h>

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
        glm::vec3 WorldSize = glm::vec3(10, 10, 10);

        /** Owned by the manager! */
        GLuint ComputeProgramMesh;
        GLuint ComputeProgramPaint;
        GLuint ComputeProgramSelect;
        
        GLuint VertexBuffer;
        GLuint NormalBuffer;
        GLuint CountBuffer;
        GLuint HeightBuffer;
        GLuint SelectionBuffer;
        static unsigned int TexSize;

        TerrainData(glm::vec3 WorldPos, GLuint PMesh, GLuint PPaint, GLuint PSelect);
        ~TerrainData() = default;
        void DispatchSelect(GLuint Mode) const;
        void DispatchPaint() const;
        void DispatchResetHeight() const;
        void DispatchGenerate();
        void CleanUp() const;
        static void Dispatch(GLuint Mode, GLuint Target);
        
        void RenderTriangles() const;
        
        void CreateCompute();
        void ApplyToSettings(TerrainShaderSettings& Settings) const;
        void UpdateComputeVars(GLuint Program) const;
        
        static unsigned int GetHeightBufferSize();
    };
}
