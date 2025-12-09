#pragma once
#include <glm/glm.hpp>

#include <glew/include/GL/glew.h>
#include "GLFW/include/GLFW/glfw3.h"

/**
 * Contains world space position/rotation information
 * and helper functions to access them
 */
class Transform
{
public:
    glm::vec3 Position;
    glm::vec3 EulerAngles;

    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

    static glm::vec4 WorldForward;
    static glm::vec4 WorldUp;
};
