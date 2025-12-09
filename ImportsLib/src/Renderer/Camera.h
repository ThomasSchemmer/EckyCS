#pragma once
#include <memory>
#include <glm/glm.hpp>

#include <glew/include/GL/glew.h>
#include "GLFW/include/GLFW/glfw3.h"
#include "Util/Projector.h"
#include "Util/Transform.h"

class Camera : public Projector
{
public:
    glm::vec3 SunPos = glm::vec3(10, 10, 10);

    void Update(float Delta);
    Camera(GLFWwindow* Window);
    virtual void OnDrawGizmos();

    void GetMouseCoords(glm::vec2& Pos) const;
    int GetKey(int Key) const;
    int GetMouse(int Key) const;

    /**
     * Maps the current mouse cursor to the intersection point at
     * a plane with origin (0,0,0) and normal (0, 1, 0)
     */
    glm::vec3 GetMouseWorldPos() const;
    glm::vec3 GetScreenWorldPos(glm::vec2 ScreenPos) const;

private:
    float MoveSpeed, RotationSpeed;
    glm::vec2 LastMousePos;
    
    void ProcessInput(float Delta);
    void ProcessMouseInput(float Delta);
    void ProcessWASDInput(float Delta);
    void ScrollCallback(double xOffset, double yOffset);
};

