#pragma once

#include <glm/glm.hpp>
#include <glew/include/GL/glew.h>
#include "GLFW/include/GLFW/glfw3.h"

class Gizmos
{
public:
    Gizmos();
    void Render();
    void AddLine(glm::vec3 P1, glm::vec3 P2);
    void AddFrustum(std::vector<glm::vec3>& Points, glm::vec3 Center, float Scale = .9f);
    void CleanUp();
    void Clear();
    
private:
    std::vector<glm::vec3> Points;
    GLuint VAO, VBO;
};
