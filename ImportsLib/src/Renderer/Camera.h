#pragma once
#include <memory>
#include <glm/glm.hpp>

#include <glew/include/GL/glew.h>
#include "GLFW/include/GLFW/glfw3.h"

using namespace std;
class Camera
{
public:
    glm::mat4 Projection, View;
    glm::vec3 Position;
    glm::vec3 SunPos = glm::vec3(10, 10, 10);

    void Update(float Delta);
    Camera(GLFWwindow* Window);
    virtual void OnDrawGizmos();

    void GetMouseCoords(glm::vec2& Pos) const;
    int GetKey(int Key) const;
    int GetMouse(int Key) const;

    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;
    glm::vec3 GetEulerAngles() const;

    glm::vec2 GetScreenExtent() const;
    glm::vec2 GetScreenScale() const;

    /**
     * Maps the current mouse cursor to the intersection point at
     * a plane with origin (0,0,0) and normal (0, 1, 0)
     */
    glm::vec3 GetMouseWorldPos() const;

private:
    GLFWwindow* Window;
    float MoveSpeed, RotationSpeed;
    float XAngle, YAngle;
    int ZoomIndex;
    glm::vec2 LastMousePos;
    
    glm::vec4 WorldForward = glm::vec4(1, 0, 0, 0);
    glm::vec4 WorldUp = glm::vec4(0, 1, 0, 0);
    vector<float> ZoomSteps = {0.025f, 0.05f,0.1f, 0.25f, 0.5f, 0.75f, 1, 1.5f, 3, 5};
    
    void ProcessInput(float Delta);
    void ProcessMouseInput(float Delta);
    void ProcessWASDInput(float Delta);
    void ScrollCallback(double xOffset, double yOffset);
};
