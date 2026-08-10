#pragma once
#include "GeometryProvider.h"

namespace EckyCS
{
    /**
     * Creates a fullscreen quad for effects to be drawn 
     */
    class FullScreenQuadProvider : public GeometryProvider
    {
    public:
        void* GetVertexArray() override
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

        // dont need to load anything
        void Load(const wchar_t* Path) override {}

        float SpriteVertices[48] = {
            // vec3 pos, vec2 uv, vec3 normal
            // --- Front face ---
            -1.0f,-1.0f, 0.0f, 0.0f, 0.0f, 0, 0, 1,
             1.0f,-1.0f, 0.0f, 1.0f, 0.0f, 0, 0, 1,
             1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0, 0, 1,

             1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0, 0, 1,
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0, 0, 1,
            -1.0f,-1.0f, 0.0f, 0.0f, 0.0f, 0, 0, 1,
        };
    };
}
