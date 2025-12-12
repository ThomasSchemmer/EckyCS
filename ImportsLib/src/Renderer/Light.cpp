#include "Light.h"

#include "Renderer.h"
#include "../GameService/Game.h"
#include "imgui/imgui.h"

using namespace std;

Light::Light(glm::vec3 InPosition, glm::vec3 InEuler, glm::vec3 InColor, GLFWwindow* InWindow, const shared_ptr<Camera>& Cam):
    Color(InColor), CameraPtr(Cam)
{
    Window = InWindow;
    Position = InPosition;
    EulerAngles = InEuler;
    Projection = glm::mat4(1.0f);
    View = glm::lookAt(Position, Position + GetForward(), GetUp());
}

void Light::OnDrawGizmos(const shared_ptr<Gizmos>& Gizmos)
{
    ImGui::Begin("Light");
    ImGui::SliderFloat3("Pos", glm::value_ptr(Position), -5, 5);
    ImGui::SliderFloat3("Dir", glm::value_ptr(EulerAngles), glm::radians(-360.0f), glm::radians(360.0f));
    
    ImGui::End();
}

void Light::Update(float delta)
{
    CalculateFrustum();
}

void Light::CalculateFrustum()
{
    // we take the WS camera frustum points
    auto WorldSpacePoints = CameraPtr->GetFrustumPoints();
    vector<glm::vec3> ProjSpacePoints;
    ProjSpacePoints.reserve(WorldSpacePoints.size());
    
    // and transform them into the projected light space
    for (const auto& WSPoint : WorldSpacePoints)
    {
        glm::vec3 ProjPoint = xyz(View * glm::vec4(WSPoint, 1));
        ProjSpacePoints.push_back(ProjPoint);
    }

    // so that we can build an AABB
    auto Min = glm::vec3(FLT_MAX);
    auto Max = glm::vec3(-FLT_MAX);
    for (const auto& ProjPoint : ProjSpacePoints)
    {
        Min = min(Min, ProjPoint);
        Max = max(Max, ProjPoint);
    }

    // since world space and ShadowMap res do not match 1:1 we get fractions
    // causing varying lookup positions depending on camera movement
    // this leads to "floating, crawling" shadow edges
    // Fix: Snap lookup coords to pixels
    constexpr float ShadowRes = 1024;
    glm::vec2 TexelSize;
    TexelSize.x = (Max.x - Min.x) / ShadowRes;
    TexelSize.y = (Max.y - Min.y) / ShadowRes;
    
    glm::vec2 LightPosInTexel = glm::vec2(Min.x, Min.y) / TexelSize;
    LightPosInTexel.x = floor(LightPosInTexel.x + 0.5f);
    LightPosInTexel.y = floor(LightPosInTexel.y + 0.5f);
    glm::vec2 SnappedMin = LightPosInTexel * TexelSize;

    // shift the projection box
    float dx = SnappedMin.x - Min.x;
    float dy = SnappedMin.y - Min.y;
    Min.x += dx;
    Max.x += dx;
    Min.y += dy;
    Max.y += dy;

    // and then use that box to get the actual light frustum position & clip planes
    auto LightPosLight = (Min + Max) / 2.0f;
    auto Inv = inverse(View);
    Position = xyz(Inv * glm::vec4(LightPosLight, 1));

    View = glm::lookAt(Position, Position + GetForward(), GetUp());
    Projection = glm::ortho(Min.x, Max.x, Min.y, Max.y, Min.z, Max.z);
    ClipPlanes = glm::vec2(Min.z, Max.z);
}
