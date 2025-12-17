#pragma once
#include "GeometryProvider.h"

namespace EckyCS
{
    /**
     * Creates a sprite mesh for Entities to be drawn
     * //TODO: make static, as a sprite is shared 
     */
    class SpriteGeometryProvider : public GeometryProvider
    {
    public:
        float* GetVertexArray() override
        {
            return SpriteVertices;
        }

        GLsizei GetVertexCount() const override
        {
            return 6;
        }

        GLsizei GetVertexByteCount() const override
        {
            return sizeof(SpriteVertices);
        }

        float SpriteVertices[48] = {
            // vec3 pos, vec2 uv, vec3 normal
            // --- Front face ---
            -0.5f,-0.5f, 0.0f, 0.0f, 0.0f, 0, 0, 1,
             0.5f,-0.5f, 0.0f, 1.0f, 0.0f, 0, 0, 1,
             0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0, 0, 1,

             0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0, 0, 1,
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0, 0, 1,
            -0.5f,-0.5f, 0.0f, 0.0f, 0.0f, 0, 0, 1,
        };
    };
}
