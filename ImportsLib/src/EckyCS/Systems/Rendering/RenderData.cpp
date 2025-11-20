#include "RenderData.h"

#include <iostream>
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>

#include "../../Components/Base/TransformComponent.h"

namespace EckyCS
{
    void RenderData::Create(size_t InCount)
    {
        Count = InCount;
        glGenVertexArrays(1, &VAO);

        glGenBuffers(1, &VerticesBuffer);
        glGenBuffers(1, &OffsetBuffer);
        glGenBuffers(1, &IDBuffer);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VerticesBuffer);
        // since the mesh is fixed we can immediately set it. Vertices, uv and normals are all in the same data block
        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 5));
        glEnableVertexAttribArray(2);
        
        
        glBindBuffer(GL_ARRAY_BUFFER, OffsetBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TransformComponent) * InCount, nullptr, GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); 
        
        glBindBuffer(GL_ARRAY_BUFFER, IDBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(EntityID) * InCount, nullptr, GL_STATIC_DRAW);
        
    }


    void RenderData::Render() const
    {
        glBindVertexArray(VAO);
        GLint vao, vbo, prog;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, static_cast<int>(Count));
    }
}
