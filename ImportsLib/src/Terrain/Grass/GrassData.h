#pragma once

#include <memory>

#include "GrassShader.h"
#include "../../EckyCS/Systems/Rendering/GeometryProvider.h"

enum class RenderPassType : uint8_t;

namespace TTerrain
{
    class GrassShader;
}

namespace TTerrain
{
    class TerrainManager;
    class TerrainData;
}

namespace TTerrain
{
    /**
     * Creates position data for the grass once with a compute shader,
     * but does not copy it back to the CPU - so its implicit only.
     * Since its not a fixed amount, we have to run twice:
     * 1) Count how many we would need, allocate enough buffer size
     * 2) Actually compute positions and write them into the buffer
     * The approach is relatively similar to @TerrainData
     * Uses Terrain Height data to check where grass should be generated
     */
    class GrassData 
    {
    public:
        GrassData(const std::shared_ptr<TerrainManager>& InManager);
        ~GrassData() = default;
        void DispatchGenerate(const TerrainData& Data);
        void CleanUp() const;
        void Render(RenderPassType Type) const;
        
    private:
        GLuint VAO = 0;
        GLuint PositionBuffer = 0;
        GLuint CountBuffer = 0;
        GLuint VertexBuffer = 0;
        GLsizei AppendCount = 0;

        // owned by the Manager!
        GLuint GrassCompute = 0;
        GLuint GrassShaderProgram = 0;

        glm::ivec2 TargetCount = glm::ivec2(100);
        unsigned int GroupCount = 8;

        std::shared_ptr<GrassShader> GrassShader;
        std::shared_ptr<TerrainManager> Manager;

        GrassShaderSettings& GetStandardSettings() const;
    };
}
