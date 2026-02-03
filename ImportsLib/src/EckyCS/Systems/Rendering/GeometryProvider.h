#pragma once

#include <glew/include/GL/glew.h>
namespace EckyCS
{
    /**
     * Describes what geometry should be rendered for the Entities
     * Can be a @MeshDataProvider, or @SpriteDataProvider
     */
    class GeometryProvider
    {
    public:
        virtual void* GetVertexArray() = 0;

        virtual GLsizei GetVertexCount() const= 0;

        virtual GLsizei GetVertexByteCount() const = 0;

        virtual void Load(const wchar_t* Path) = 0;

        virtual ~GeometryProvider() = default;
    };
}