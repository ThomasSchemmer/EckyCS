#include "Camera.h"

#include <glfw/include/GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>

#include "imgui/imgui.h"

void Camera::Update(float Delta)
{
    ProcessInput(Delta);

    glm::vec3 Forward;
    Forward.x = cos(Angle);
    Forward.y = 0;
    Forward.z = sin(Angle);
    Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
    View = glm::lookAt(Position, Position + Forward, glm::xyz(CamUp));
}

Camera::Camera(const std::shared_ptr<GLFWwindow>& InWindow) :
    Position(200, 0, 400), MoveSpeed(25), RotationSpeed(0.5), Angle(glm::radians(-120.0)), Distance(3)
{
    Window = InWindow;
    Projection = glm::mat4(1.0f);
    View = glm::mat4(1.0f);
}

void Camera::OnDrawGizmos()
{
    ImGui::Begin("Transform");
    ImGui::Text("Pos: %.2f|%.2f|%.2f", Position.x, Position.y, Position.z);
    ImGui::Text("Rot: %.2f|%.2f|%.2f", glm::degrees(Angle), 0.0, 0.0);
    ImGui::End();
}



void Camera::GetMouseCoords(glm::vec2& Pos) const
{
    double MouseX, MouseY;
    glfwGetCursorPos(Window.get(), &MouseX, &MouseY);
    Pos.x = static_cast<float>(MouseX);
    Pos.y = static_cast<float>(MouseY);
}

int Camera::GetKey(int Key) const
{
    return glfwGetKey(Window.get(), Key);
}


void Camera::ProcessInput(float Delta)
{
    glm::vec3 Forward, Right;
    Forward.x = cos(Angle);
    Forward.y = 0;
    Forward.z = sin(Angle);
    Right.x = cos(Angle + static_cast<float>(glm::radians(90.0)));
    Right.y = 0;
    Right.z = sin(Angle + static_cast<float>(glm::radians(90.0)));
    
    Position += GetKey(GLFW_KEY_W) == GLFW_PRESS ?
        Delta * MoveSpeed * Forward : glm::vec3(0);
    Position -= GetKey(GLFW_KEY_S) == GLFW_PRESS ?
        Delta * MoveSpeed * Forward : glm::vec3(0);
    Position -= GetKey(GLFW_KEY_A) == GLFW_PRESS ?
        Delta * MoveSpeed * Right : glm::vec3(0);
    Position += GetKey(GLFW_KEY_D) == GLFW_PRESS ?
        Delta * MoveSpeed * Right : glm::vec3(0);

    Angle -= GetKey(GLFW_KEY_Q) == GLFW_PRESS ?
        Delta * RotationSpeed : 0;
    Angle += GetKey(GLFW_KEY_E) == GLFW_PRESS ?
        Delta * RotationSpeed : 0;
}
