#pragma once
#include <memory>

#include "GL/glew.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace TTerrain
{
    class TerrainData;
    class TerrainManager;
}

namespace TTerrain
{
    /**
     * Helper class providing functionality to get information about the terrain from the GPU
     * Queries once every frame (if dirty) per @TerrainData
     * Currently can interpolate height by world position
     * Note: Due to the async nature of the GPU we request an update and can only receive it ~2 frames later!
     */
    class TerrainHelper
    {
    public:
        /**
         * Returns the interpolated terrain height for any world position
         * Copies the provided buffers into the CPU for lookup
         */
        static float GetHeightFromWorldPos(glm::vec3 WorldPos, glm::vec2 WorldSize);

        static unsigned int GetClosestHeightData(glm::vec3 WorldPos, glm::vec2 WorldSize);

        static void Update(TerrainManager* TerrainManager);

        static void GetQuadIndices(glm::vec3 WorldPos, glm::vec2 WorldSize, int& QuadIndex, glm::ivec2& QuadXY);
        
    private:
        static void UpdateData(TerrainManager* TerrainManager);
        static void DeleteFence(int i);
        
        static int GetQuadIndexFor(glm::vec3 id, glm::ivec2 Offset);
        static std::vector<glm::vec3> GetClosestVertices(glm::vec3 WorldPos, glm::vec2 WorldSize);
        static float GetSideOfTriangle(int IndexInQuad, int StartVertex, glm::vec3 WorldPos, const std::vector<glm::vec4>& Vertices);
        static glm::vec3 GetBaricentricCoordinates(glm::vec3 WorldPos, const glm::vec3& VertA, const glm::vec3& VertB, const glm::vec3& VertC);

        static std::vector<std::tuple<unsigned int, GLsync>> Fences;
        static std::vector<glm::vec4> Vertices;
        static std::vector<int> Offsets;
        static std::vector<unsigned int> HeightDatas;
    };
}
