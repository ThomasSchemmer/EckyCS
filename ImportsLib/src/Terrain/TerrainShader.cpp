#include "TerrainShader.h"
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>
#include "../Renderer/Camera.h"
#include "../Util/ShaderHelper.h"
using namespace Util;

TTerrain::TerrainShader::TerrainShader()
{
    string VertexCode = ShaderHelper::LoadShader(VertexShaderPath);
    string FragmentCode = ShaderHelper::LoadShader(FragmentShaderPath);

    unsigned int Vertex = ShaderHelper::CompileShader(VertexCode, GL_VERTEX_SHADER);
    unsigned int Fragment = ShaderHelper::CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    vector IDs = {Vertex, Fragment};
    Program = ShaderHelper::CreateProgram(IDs);
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    
    Transform = glm::mat4(1.0f);
}

TTerrain::TerrainShader::~TerrainShader()
{
}

void TTerrain::TerrainShader::Use() const
{
    glUseProgram(Program);
}

void TTerrain::TerrainShader::UpdateVars(const shared_ptr<Camera>& Camera) const
{
    auto BrushPos = GetMouseWorldPos(Camera);
    ShaderHelper::SetUniform3fv("BrushPos", BrushPos, Program);
    ShaderHelper::SetUniformM4("Transform", Transform, Program);
    ShaderHelper::SetUniformM4("Projection", Camera->Projection, Program);
    ShaderHelper::SetUniformM4("View", Camera->View, Program);
}

glm::vec3 TTerrain::TerrainShader::GetMouseWorldPos(const shared_ptr<Camera>& Camera)
{
    // we need to convert from pixel to (zoomed-in) world space
    // translate mouse position to world space
    glm::vec2 MouseScreenPos; 
    Camera->GetMouseCoords(MouseScreenPos);
    
    const auto ScreenExtend = Camera->GetScreenExtent();
    const auto ScreenScale = Camera->GetScreenScale();
    const auto ScreenExtendScaled = ScreenExtend / ScreenScale;
    const float HScale = ScreenExtend.x / ScreenExtend.y;

    // we need to adjust for screen width/height relation
    auto MouseWorldPos = (MouseScreenPos - ScreenExtendScaled) * ScreenScale;
    MouseWorldPos.y *= 1 + (HScale - 1) / 2.0f;

    // rotate the once-screen vector based on camera angle
    auto Temp = glm::vec3(MouseWorldPos.x, 0, MouseWorldPos.y);
    float YAngle = Camera->GetEulerAngles().y; 
    glm::mat4 R = glm::rotate(glm::mat4(1.f), YAngle, glm::vec3(0,1,0));
    auto MousePosRotated = glm::vec3(R * glm::vec4(Temp, 1.f));
    
    // now that its translated to world space we can add the camera offset to it
    auto Origin = Camera->Position;
    auto Dir = Camera->GetForward();
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
