#pragma once
#include "GL/glew.h"
#include "../../Renderer/Passes/RenderPass.h"

namespace TTerrain
{
    class TerrainData;

    /**
     * Helper class to create water meshes
     * Similar to @TerrainData, but doesn't use height information, only checks if
     * water should be created or not
     */
    class WaterData
    {
    public:
        WaterData();
        ~WaterData();
        
        void DispatchGenerate(TerrainData& Data);
        void Dispatch(GLuint Mode, GLuint Target);
        void Render(RenderPassType Type) const;

    private:
        GLuint CountBuffer = 0;
        GLuint VertexBuffer = 0;
        GLuint VertexOffsetBuffer = 0;

        GLsizei AppendCount = 0;

        void CreateTempCompute();
        void CreateCompute();
        
        
        static constexpr unsigned int LAYOUT_WATER = 1 << 25;
    };
}
