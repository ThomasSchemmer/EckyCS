#pragma once
#include "GL/glew.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace TTerrain
{
    class TerrainHelper
    {
    public:
        /**
         * Returns the interpolated terrain height for any world position
         * Copies the provided buffers into the CPU for lookup
         * TODO: make efficient
         */
        static float GetHeightFromWorldPos(glm::vec3 WorldPos, GLuint VertexBuffer, GLuint VertexOffsetBuffer, glm::vec2 WorldSize);

    private:
        static int GetQuadIndexFor(glm::vec3 id, glm::ivec2 Offset);
        static float GetSideOfTriangle(int IndexInQuad, int StartVertex, glm::vec3 WorldPos, const std::vector<glm::vec4>& Vertices);
        static glm::vec3 GetBaricentricCoordinates(glm::vec3 WorldPos, const glm::vec4& VertA, const glm::vec4& VertB, const glm::vec4& VertC);
    };
}
