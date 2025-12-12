#include "Camera.h"

#include <algorithm>
#include <glfw/include/GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>

#include "Gizmos.h"
#include "imgui/imgui.h"
using namespace std;

void Camera::Update(float Delta)
{
    ProcessInput(Delta);
    auto Screen = GetScreenExtent();
    UpdateProjection(Screen);
}

Camera::Camera(GLFWwindow* InWindow) :
    MoveSpeed(250), RotationSpeed(1),   
    LastMousePos(glm::vec2(0,0))
{
    Window = InWindow;
    ClipPlanes = glm::vec2(-100, 100);
    Projection = glm::mat4(1.0f);
    View = glm::mat4(1.0f);
    EulerAngles.x = glm::radians(45.0f);
    EulerAngles.y = glm::radians(45.0f);
    
    auto Origin = glm::vec3(0, 0, 0);
    auto Dir = normalize(glm::vec3(0.35f, 0.7f, 0.61f));
    Position = Origin + 1.0f * Dir;

    glfwSetWindowUserPointer(Window, this);
    glfwSetScrollCallback(Window,
    [](GLFWwindow* win, double xOffset, double yOffset)
    {
        auto Cam = static_cast<Camera*>(glfwGetWindowUserPointer(win));
        Cam->ScrollCallback(xOffset, yOffset);
    });
}

void Camera::OnDrawGizmos(const shared_ptr<Gizmos>& Gizmos)
{
    auto Forward = GetForward();
    ImGui::Begin("Transform");
    ImGui::Text("Pos: %.2f|%.2f|%.2f", Position.x, Position.y, Position.z);
    ImGui::Text("Rot: %.2f|%.2f|%.2f\n", glm::degrees(EulerAngles.x), glm::degrees(EulerAngles.y), EulerAngles.z);
    ImGui::Text("For: %.2f|%.2f|%.2f\n", Forward.x, Forward.y, Forward.z);
    ImGui::End();
}

void Camera::GetMouseCoords(glm::vec2& Pos) const
{
    double MouseX, MouseY;
    glfwGetCursorPos(Window, &MouseX, &MouseY);
    Pos.x = static_cast<float>(MouseX);
    Pos.y = static_cast<float>(MouseY);
}

int Camera::GetKey(int Key) const
{
    return glfwGetKey(Window, Key);
}

int Camera::GetMouse(int Key) const
{
    return glfwGetMouseButton(Window, Key);
}

void Camera::ProcessInput(float Delta)
{
    ProcessMouseInput(Delta);

    ProcessWASDInput(Delta);
}

void Camera::ProcessWASDInput(float Delta)
{
    constexpr auto Forward = glm::vec3(1, 0, 1);
    constexpr auto Right = glm::vec3(1, 0, -1);

    auto Zoom = ZoomSteps[ZoomIndex];
    Position -= GetKey(GLFW_KEY_W) == GLFW_PRESS ?
        Zoom * Delta * MoveSpeed * Forward : glm::vec3(0);
    Position += GetKey(GLFW_KEY_S) == GLFW_PRESS ?
        Zoom * Delta * MoveSpeed * Forward : glm::vec3(0);
    Position -= GetKey(GLFW_KEY_A) == GLFW_PRESS ?
        Zoom * Delta * MoveSpeed * Right : glm::vec3(0);
    Position += GetKey(GLFW_KEY_D) == GLFW_PRESS ?
        Zoom * Delta * MoveSpeed * Right : glm::vec3(0);
}

void Camera::ProcessMouseInput(float Delta)
{
    static bool bWasRightMouseDown = false;
    const bool bIsRightMouseDown = GetMouse(GLFW_MOUSE_BUTTON_RIGHT);
    const bool bIsRightMouseDownNow = !bWasRightMouseDown && bIsRightMouseDown;

    double MouseX, MouseY;
    glfwGetCursorPos(Window, &MouseX, &MouseY);
     
    if (bIsRightMouseDownNow)
    {
        LastMousePos = glm::vec2(MouseX, MouseY);
    }
    if (bIsRightMouseDown)
    {
        EulerAngles.x -= (static_cast<float>(MouseY) - LastMousePos.y) * RotationSpeed * Delta;
        EulerAngles.y += (static_cast<float>(MouseX) - LastMousePos.x) * RotationSpeed * Delta;
        LastMousePos = glm::vec2(MouseX, MouseY);
    }
    
    bWasRightMouseDown = bIsRightMouseDown;
}

void Camera::ScrollCallback(double xOffset, double yOffset)
{
    ZoomIndex = static_cast<int>(round(ZoomIndex + yOffset));
    ZoomIndex = clamp(ZoomIndex, 0, static_cast<int>(ZoomSteps.size()) - 1);
}


glm::vec3 Camera::GetMouseWorldPos() const
{
    glm::vec2 MouseScreenPos; 
    GetMouseCoords(MouseScreenPos);
    return GetScreenWorldPos(MouseScreenPos);
}

glm::vec3 Camera::GetScreenWorldPos(glm::vec2 ScreenPos) const
{
    // we need to convert from pixel to (zoomed-in) world space
    // translate mouse position to world space
    const auto ScreenExtend = GetScreenExtent();
    const auto ScreenScale = GetScreenScale();
    const auto ScreenExtendScaled = ScreenExtend / ScreenScale;
    const float HScale = ScreenExtend.x / ScreenExtend.y;

    // we need to adjust for screen width/height relation
    auto MouseWorldPos = (ScreenPos - ScreenExtendScaled) * ScreenScale;
    MouseWorldPos.y *= 1 + (HScale - 1) / 2.0f;

    // rotate the once-screen vector based on camera angle
    auto Temp = glm::vec3(MouseWorldPos.x, 0, MouseWorldPos.y);
    glm::mat4 R = glm::rotate(glm::mat4(1.f), EulerAngles.y, glm::vec3(0,1,0));
    auto MousePosRotated = glm::vec3(R * glm::vec4(Temp, 1.f));
    
    // now that it's translated to world space we can add the camera offset to it
    auto Origin = Position;
    auto Dir = GetForward();
    auto PlaneOrigin = glm::vec3(0,0,0);
    auto PlaneNormal = glm::vec3(0, 1, 0);

    // Parametric intersection with plane at (0,0,0)
    float Denom = dot(PlaneNormal, Dir);
    float t = dot(PlaneOrigin - Origin, PlaneNormal) / Denom;
    auto Pos = t * Dir + Origin;
    Pos.x += MousePosRotated.x;
    Pos.z += MousePosRotated.z;
    return Pos;
}

