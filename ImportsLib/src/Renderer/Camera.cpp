#include "Camera.h"

#include <glfw/include/GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

void Camera::Update()
{
    auto Time = glfwGetTime();
    auto CamPosX = sin(Time) * 10;
    auto CamPosY = cos(Time) * 10;
    auto CamPos = glm::vec3(CamPosX, 0, CamPosY);
    auto CamFront = glm::vec3(0, 0, -1);

    auto CamUp = glm::vec4(0, 1, 0, 0);
    
    Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    View = glm::lookAt(CamPos, glm::vec3(0,0,0), xyz(CamUp));
}

Camera::Camera(const std::function<int(int)>& InputCallback) : MoveSpeed(5)
{
    this->InputCallback = InputCallback;
    Projection = glm::mat4(1.0f);
    View = glm::mat4(1.0f);
}

void Camera::ProcessInput()
{
    if (InputCallback(GLFW_KEY_W) == GLFW_PRESS)
    {
        Position.z -= MoveSpeed;
    }
    if (InputCallback(GLFW_KEY_S) == GLFW_PRESS)
    {
        Position.z += MoveSpeed;
    }
}
