#include "Camera.h"

#include <algorithm>
#include <glfw/include/GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>

#include "imgui/imgui.h"

void Camera::Update(float Delta)
{
    ProcessInput(Delta);
    auto Screen = GetScreenExtent();
    Projection = glm::ortho(-Screen.x, Screen.x, -Screen.y, Screen.y, -10.0f, 1000.0f);
    View = lookAt(Position, Position + GetForward(), GetUp());
}

Camera::Camera(const std::shared_ptr<GLFWwindow>& InWindow) :
    MoveSpeed(250), RotationSpeed(1),
    XAngle(glm::radians(45.0f)), YAngle(glm::radians(45.0f)),   
    LastMousePos(glm::vec2(0,0)), ZoomIndex(4)
{
    Window = InWindow;
    Projection = glm::mat4(1.0f);
    View = glm::mat4(1.0f);
    
    auto Origin = glm::vec3(0, 0, 0);
    auto Dir = glm::vec3(0.35f, 0.7f, 0.61f);
    Position = Origin + 100.0f * Dir;

    glfwSetWindowUserPointer(Window.get(), this);
    glfwSetScrollCallback(Window.get(),
    [](GLFWwindow* win, double xOffset, double yOffset)
    {
        auto Cam = static_cast<Camera*>(glfwGetWindowUserPointer(win));
        Cam->ScrollCallback(xOffset, yOffset);
    }
);
}

void Camera::OnDrawGizmos()
{
    auto Forward = GetForward();
    ImGui::Begin("Transform");
    ImGui::Text("Pos: %.2f|%.2f|%.2f", Position.x, Position.y, Position.z);
    ImGui::Text("Rot: %.2f|%.2f|%.2f\n", glm::degrees(XAngle), glm::degrees(YAngle), 0.0);
    ImGui::Text("For: %.2f|%.2f|%.2f\n", Forward.x, Forward.y, Forward.z);
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

int Camera::GetMouse(int Key) const
{
    return glfwGetMouseButton(Window.get(), Key);
}

glm::vec3 Camera::GetForward() const
{
    glm::vec3 Forward;
    Forward.x = cos(XAngle) * cos(YAngle);
    Forward.y = sin(XAngle);
    Forward.z = cos(XAngle) * sin(YAngle);
    return -normalize(Forward);
}

glm::vec3 Camera::GetRight() const
{
    return normalize(cross(GetForward(), xyz(WorldUp)));
}

glm::vec3 Camera::GetUp() const
{
    return normalize(-cross(GetForward(), GetRight()));
}

glm::vec3 Camera::GetEulerAngles() const
{
    return glm::vec3(XAngle, YAngle, 0.0f);
}

glm::vec2 Camera::GetScreenExtent() const
{
    int Width, Height;
    glfwGetFramebufferSize(Window.get(), &Width, &Height);
    auto Scale = GetScreenScale();
    float W = static_cast<float>(Width) / 2 * Scale.x;
    float H = static_cast<float>(Height) / 2 * Scale.y;
    return glm::vec2(W, H);
}

glm::vec2 Camera::GetScreenScale() const
{
    return {ZoomSteps[ZoomIndex], ZoomSteps[ZoomIndex]}; 
}

void Camera::ProcessInput(float Delta)
{
    ProcessMouseInput(Delta);

    ProcessWASDInput(Delta);
}

void Camera::ProcessWASDInput(float Delta)
{
    const auto Forward = glm::vec3(1, 0, 1);
    const auto Right = glm::vec3(1, 0, -1);
    const auto Up = GetUp();

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
    glfwGetCursorPos(Window.get(), &MouseX, &MouseY);
     
    if (bIsRightMouseDownNow)
    {
        LastMousePos = glm::vec2(MouseX, MouseY);
    }
    if (bIsRightMouseDown)
    {
        XAngle -= (static_cast<float>(MouseY) - LastMousePos.y) * RotationSpeed * Delta;
        YAngle += (static_cast<float>(MouseX) - LastMousePos.x) * RotationSpeed * Delta;
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
    // we need to convert from pixel to (zoomed-in) world space
    // translate mouse position to world space
    glm::vec2 MouseScreenPos; 
    GetMouseCoords(MouseScreenPos);
    
    const auto ScreenExtend = GetScreenExtent();
    const auto ScreenScale = GetScreenScale();
    const auto ScreenExtendScaled = ScreenExtend / ScreenScale;
    const float HScale = ScreenExtend.x / ScreenExtend.y;

    // we need to adjust for screen width/height relation
    auto MouseWorldPos = (MouseScreenPos - ScreenExtendScaled) * ScreenScale;
    MouseWorldPos.y *= 1 + (HScale - 1) / 2.0f;

    // rotate the once-screen vector based on camera angle
    auto Temp = glm::vec3(MouseWorldPos.x, 0, MouseWorldPos.y);
    glm::mat4 R = glm::rotate(glm::mat4(1.f), YAngle, glm::vec3(0,1,0));
    auto MousePosRotated = glm::vec3(R * glm::vec4(Temp, 1.f));
    
    // now that its translated to world space we can add the camera offset to it
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
