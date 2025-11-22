#include "TerrainManager.h"

#include "TerrainShader.h"
#include "../EckyCS/ECS.h"
#include "../GameService/Game.h"
#include "../Util/ShaderHelper.h"
#include "imgui/imgui.h"
#include "../Renderer/BaseShader.h"
using namespace Util;

TTerrain::TerrainManager::TerrainManager(const shared_ptr<Camera>& InCamPtr)
{
    CamPtr = InCamPtr;
    Shader = make_shared<TerrainShader>();
    CreateCompute();
    CreateMesh();
}

void TTerrain::TerrainManager::Render() const
{
    Shader->Use();
    Shader->UpdateVars(CamPtr);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void TTerrain::TerrainManager::Dispatch() const
{
    // execute terrain gen
    glUseProgram(ComputeProgram);
    glBindImageTexture(1, ResultTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glDispatchCompute(256 / 8, 256 / 8, 1);
    //clear binding to make the tex displayable in UI
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    
}

void TTerrain::TerrainManager::OnDrawGizmos() const
{
    auto Pos = TerrainShader::GetMouseWorldPos(CamPtr);
    ImGui::Begin("OpenGL Texture Text");
    ImGui::Text("Pos: %.2f|%.2f|%.2f", Pos.x, Pos.y, Pos.z);
    ImGui::Image((ImTextureID)(intptr_t)ResultTex, ImVec2(256, 256));
    ImGui::End();
}

void TTerrain::TerrainManager::CreateMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VertexBuffer);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PlaneVertices), PlaneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    GLuint OffsetBuffer;
    float Pos[] = { -50, 0, -50 };
    glGenBuffers(1, &OffsetBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, OffsetBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Pos), Pos, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
}

void TTerrain::TerrainManager::CreateCompute()
{
    string ComputeCode = ShaderHelper::LoadShader(ComputePath);
    unsigned int Compute = ShaderHelper::CompileShader(ComputeCode, GL_COMPUTE_SHADER);

    vector IDs = {Compute};
    ComputeProgram = ShaderHelper::CreateProgram(IDs);
    glDeleteShader(Compute);

    glGenTextures(1, &ResultTex);
    glBindTexture(GL_TEXTURE_2D, ResultTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 256, 256, 0,GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
}



TTerrain::TerrainManager::~TerrainManager()
{
    //todo: here and shader: kill program if valid
}

