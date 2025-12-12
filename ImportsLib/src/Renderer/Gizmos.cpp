#include "Gizmos.h"

Gizmos::Gizmos()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
}

void Gizmos::Render()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, Points.size() * sizeof(glm::vec3), Points.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINES, 0, Points.size());
    Clear();
}

void Gizmos::AddLine(glm::vec3 P1, glm::vec3 P2)
{
    Points.emplace_back(P1);
    Points.emplace_back(P2);
    
}

void Gizmos::AddFrustum(std::vector<glm::vec3>& FrustumPoints, glm::vec3 Center, float Scale /*= .9f*/)
{
    for (auto& Point : FrustumPoints)
    {
        Point -= Center;
        Point *= Scale;
        Point += Center;
    }
    // near plane
    AddLine(FrustumPoints[0], FrustumPoints[1]);
    AddLine(FrustumPoints[1], FrustumPoints[2]);
    AddLine(FrustumPoints[2], FrustumPoints[3]);
    AddLine(FrustumPoints[3], FrustumPoints[0]);

    // far plane
    AddLine(FrustumPoints[4], FrustumPoints[5]);
    AddLine(FrustumPoints[5], FrustumPoints[6]);
    AddLine(FrustumPoints[6], FrustumPoints[7]);
    AddLine(FrustumPoints[7], FrustumPoints[4]);

    //sides
    AddLine(FrustumPoints[0], FrustumPoints[4]);
    AddLine(FrustumPoints[1], FrustumPoints[5]);
    AddLine(FrustumPoints[2], FrustumPoints[6]);
    AddLine(FrustumPoints[3], FrustumPoints[7]);
}

void Gizmos::CleanUp()
{
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void Gizmos::Clear()
{
    Points.clear();
}
