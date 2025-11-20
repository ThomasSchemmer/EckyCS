#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

using namespace std;
class Camera
{
public:
    glm::mat4 Projection, View;
    glm::vec3 Position;
    glm::vec3 SunPos = glm::vec3(10, 10, 10);

    void Update(float Delta);
    Camera(const std::shared_ptr<GLFWwindow>& Window);
    virtual void OnDrawGizmos();

    void GetMouseCoords(glm::vec2& Pos) const;
    int GetKey(int Key) const;

private:
    std::shared_ptr<GLFWwindow> Window;
    float MoveSpeed;
    float RotationSpeed;
    float Angle;
    float Distance;
    glm::vec4 CamForward = glm::vec4(1, 0, 0, 0);
    glm::vec4 CamUp = glm::vec4(0, 1, 0, 0);
    
    void ProcessInput(float Delta);
};
