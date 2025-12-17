#include "RenderData.h"

#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>

#include "GeometryProvider.h"

namespace EckyCS
{
    void RenderData::Create(size_t InCount, const shared_ptr<GeometryProvider>& Provider)
    {
        DataProvider = Provider;
        
        Count = InCount;
        glGenVertexArrays(1, &VAO);

        glGenBuffers(1, &VerticesBuffer);
        glGenBuffers(1, &OffsetBuffer);
        glGenBuffers(1, &IDBuffer);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VerticesBuffer);
        // since the mesh is fixed we can immediately set it. Vertices, uv and normals are all in the same data block
        glBufferData(GL_ARRAY_BUFFER, DataProvider->GetVertexByteCount(), DataProvider->GetVertexArray(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 5));
        glEnableVertexAttribArray(2);

        // offsets on the other hand will have to be transmitted every frame
        glBindBuffer(GL_ARRAY_BUFFER, OffsetBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TransformComponent) * InCount, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); 
        
        glBindBuffer(GL_ARRAY_BUFFER, IDBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(EntityID) * InCount, nullptr, GL_DYNAMIC_DRAW);
        
    }

    void RenderData::Render() const
    {
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, DataProvider->GetVertexCount(), static_cast<int>(Count));
    }
}
