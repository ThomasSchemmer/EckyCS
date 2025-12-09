#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glew/include/GL/glew.h>

#include "Transform.h"
#include "GLFW/include/GLFW/glfw3.h"

/**
 * Contains information and helper functions that allow screen projection
 * Useful for anything that renders, like @Camera or @Light
 */
class Projector : public Transform
{
public:
    glm::mat4 Projection, View;
    GLFWwindow* Window;
    glm::vec2 ClipPlanes;

    void UpdateProjection(glm::vec2 Screen);

    /** Returns the current screen width/height divided by 2, depending on the zoom factor */
    glm::vec2 GetScreenExtent() const;

    static glm::vec2 GetScreenScale();
    glm::vec2 GetScreenSize() const;

    /** Calculates the world space locations of the 8 frustum corners*/
    std::vector<glm::vec3> GetFrustumPoints() const;

    static int ZoomIndex;
    static std::vector<float> ZoomSteps;
};
