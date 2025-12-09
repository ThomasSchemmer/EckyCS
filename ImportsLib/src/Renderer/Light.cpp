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
    View = glm::mat4(
        glm::vec4(GetRight(),   0),
        glm::vec4(GetUp(),      0),
        glm::vec4(GetForward(), 0),
        glm::vec4(0,0,0,1)
    );
    View = glm::transpose(View);

    // todo: delete
    Position = glm::vec3(-5, 0, 0);
    EulerAngles = glm::vec3(glm::radians(45.0f), glm::radians(180.0f), 0);
    auto Tmp = GetRight();
    
    Projection = glm::mat4(1.0f);
    View = glm::lookAt(Position, Position + GetForward(), GetUp());
}

void Light::OnDrawGizmos()
{
    ImGui::Begin("Light");
    ImGui::SliderFloat3("Pos", glm::value_ptr(Position), -5, 5);
    ImGui::SliderFloat3("Dir", glm::value_ptr(EulerAngles), glm::radians(-360.0f), glm::radians(360.0f));
    
    ImGui::End();
}

void Light::Update(float delta)
{
    auto Screen = GetScreenExtent();
    auto Size = GetScreenSize();
    //Position = CameraPtr->GetScreenWorldPos(Size / 2.0f);
    //ClipPlanes = glm::vec2(-100, 1000);
    //UpdateProjection(Screen * 2.0f);

    
    // todo: make frustum calculation correct and adopt clip planes
    CalculateFrustum();
    
    /*
    glm::vec4 origin = View * glm::vec4(0,0,0,1);
    xyz(origin) /= origin.w;
    xyz(origin) = xyz(origin) * 0.5f + 0.5f;

    glm::vec2 shadowTexelSize = glm::vec2(1.0f / 1024.0f);

    glm::vec2 snapped(
        floor(origin.x / shadowTexelSize.x) * shadowTexelSize.x,
        floor(origin.y / shadowTexelSize.y) * shadowTexelSize.y
    );

    float dx = snapped.x - origin.x;
    float dy = snapped.y - origin.y;

    View[3][0] += dx * 2.0f;
    View[3][1] += dy * 2.0f;
    */
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

    // so that we can build an AABB bounding box
    
    auto Min = glm::vec3(FLT_MAX);
    auto Max = glm::vec3(-FLT_MAX);
    for (const auto& ProjPoint : ProjSpacePoints)
    {
        Min = min(Min, ProjPoint);
        Max = max(Max, ProjPoint);
    }

    //float Pad = .5f;
    //Min -= glm::vec3(Pad);
    //Max += glm::vec3(Pad);

    // and then use that box to get the actual light frustum position & clip planes
    auto LightPosLight = (Min + Max) / 2.0f;
    Position = xyz(inverse(View) * glm::vec4(LightPosLight, 1));
    Position = Position - GetForward() * 5.0f;
    View[3] = glm::vec4(-Position, 1.0f);
    Projection = glm::ortho(Min.x, Max.x, Min.y, Max.y, Min.z, Max.z);
}
