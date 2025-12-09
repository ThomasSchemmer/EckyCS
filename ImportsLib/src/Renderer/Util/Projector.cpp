#include "Projector.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>


int Projector::ZoomIndex = 3;
std::vector<float> Projector::ZoomSteps = {0.005f, 0.01f, 0.025f, 0.05f, 0.1f, 0.15f};

void Projector::UpdateProjection(glm::vec2 Screen)
{
    Projection = glm::ortho(-Screen.x, Screen.x, -Screen.y, Screen.y, ClipPlanes.x, ClipPlanes.y);
    View = lookAt(Position, Position + GetForward(), GetUp());
}

glm::vec2 Projector::GetScreenExtent() const
{
    auto Size = GetScreenSize();
    auto Scale = GetScreenScale();
    float W = Size.x / 2 * Scale.x;
    float H = Size.y / 2 * Scale.y;
    return {W, H};
}

std::vector<glm::vec3> Projector::GetFrustumPoints() const
{
    // extrapolate the four corners of the screen into near and far plane
    std::vector<glm::vec3> FrustumPoints;
    auto Extend = GetScreenExtent();

    auto Right = GetRight();
    auto Up = GetUp();
    auto Forward = GetForward();
    auto Near = Forward * ClipPlanes.x;
    auto Far = Forward * ClipPlanes.y;
    glm::vec3 TopRight = Position + Right * Extend.x + Up * Extend.y;
    glm::vec3 BottomRight = Position + Right * Extend.x - Up * Extend.y;
    glm::vec3 BottomLeft = Position - Right * Extend.x - Up * Extend.y;
    glm::vec3 TopLeft = Position - Right * Extend.x + Up * Extend.y;

    auto InvView = glm::inverse(View);
    auto TRN = TopRight + Near;
    auto BRN = BottomRight + Near;
    auto BLN = BottomLeft + Near;
    auto TLN = TopLeft + Near;
    auto TRF = TopRight + Far;
    auto BRF = BottomRight + Far;
    auto BLF = BottomLeft + Far;
    auto TLF = TopLeft + Far;
    
    FrustumPoints.emplace_back(TRN);
    FrustumPoints.emplace_back(BRN);
    FrustumPoints.emplace_back(BLN);
    FrustumPoints.emplace_back(TLN);
    
    FrustumPoints.emplace_back(TRF);
    FrustumPoints.emplace_back(BRF);
    FrustumPoints.emplace_back(BLF);
    FrustumPoints.emplace_back(TLF);
    return FrustumPoints;
}

glm::vec2 Projector::GetScreenScale()
{
    return {ZoomSteps[ZoomIndex], ZoomSteps[ZoomIndex]}; 
}

glm::vec2 Projector::GetScreenSize() const
{
    int Width, Height;
    glfwGetFramebufferSize(Window, &Width, &Height);
    return {Width, Height};
}

