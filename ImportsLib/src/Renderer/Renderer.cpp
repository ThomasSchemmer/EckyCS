

#include <filesystem>
#include <fstream>
#include <iostream>
#include "BaseShader.h"

#include <glew/include/GL/glew.h>
#define GLFW_INCLUDE_NONE
#include "glfw/include/GLFW/glfw3.h"
#include "Renderer.h"

#include "DepthShader.h"
#include "../../../ext/imgui/imgui.h"
#include "../EckyCS/ECS.h"
#include "../EckyCS/Systems/Rendering/EntityRenderSystem.h"
#include "../GameService/Game.h"
#include "../Terrain/TerrainManager.h"
#include "Passes/BasePass.h"
#include "Passes/ShadowPass.h"
#include "../Scene/SceneManager.h"
#include "Passes/DepthPrePass.h"

using namespace EckyCS;
using namespace Scene;


void Renderer::Init(GLFWwindow* Window)
{
    ShaderPtr = make_shared<BaseShader>();
    ShaderPtr->Create();
    DepthShaderPtr = make_shared<DepthShader>();
    DepthShaderPtr->Create();
    Camera = make_shared<class Camera>(Window);
    SceneManagerPtr = make_shared<Scene::SceneManager>();
    LightPtr = make_shared<Light>(
        glm::vec3(10),
        glm::vec3(glm::radians(125.0f), glm::radians(140.0f), 0),
        glm::vec3(1),
        Window,
        Camera
    );
    GizmosPtr = make_shared<Gizmos>();

    InitRenderPasses(Window);
    SceneManagerPtr->Init();
}

void Renderer::InitRenderPasses(GLFWwindow* Window)
{
    auto SPass = make_shared<ShadowPass>();
    SPass->Create(Window);
    auto BPass = make_shared<BasePass>();
    BPass->Create(Window);
    auto DPass = make_shared<DepthPrePass>();
    DPass->Create(Window);
    RenderPasses.emplace_back(SPass);
    RenderPasses.emplace_back(DPass);
    RenderPasses.emplace_back(BPass);
}

void Renderer::Update(float Delta) const
{
    Camera->Update(Delta);
    LightPtr->Update(Delta);
}

void Renderer::Render()
{
    auto ServicePtr = Game::GetService(GameServiceType::EntityComponentSystem);
    auto Ecs = reinterpret_pointer_cast<ECS>(ServicePtr);
    vector<shared_ptr<System>> Systems;
    Ecs->TryGetSystems<BaseRenderSystem>(OUT Systems);
    
    for (auto& Pass : RenderPasses)
    {
        auto TypeStr = std::string("Render::")+ToString(Pass->Type);
        CPU_PROFILE(Game::CpuProfilerFrame, TypeStr.c_str(), legit::Colors::pumpkin);
        GPU_PROFILE(Game::GetGpuFrame(), TypeStr.c_str(), legit::Colors::pumpkin);
        
        Pass->Use();
        CurrentRenderPass = Pass;

        // terrain has its own shaders, so do it before the global ones
        Game::Instance->TerrainPtr->Render(Pass->Type);

        auto CurrentShader = GetShaderForCurrentPass();
        if (CurrentShader)
        {
            CurrentShader->Use();
            CurrentShader->UpdateVars(Camera, LightPtr);
        
            for (auto& System : Systems)
            {
                auto RenderSystem = dynamic_pointer_cast<BaseRenderSystem>(System);
                if (!RenderSystem || !RenderSystem->SupportsRenderPass(Pass->Type))
                    continue;
            
                GPU_PROFILE(Game::GetGpuFrame(), "RenderSystem", legit::Colors::silver);   
                RenderSystem->Render();
            }
        
            if (Pass->Type == RenderPassType::BasePass)
            {
                GPU_PROFILE(Game::GetGpuFrame(), "RenderGizmos", legit::Colors::silver);
                GizmosPtr->Render();
            }
        }

        Pass->OnAfterRender();

        CurrentRenderPass = nullptr;
        Pass->UnUse();
    }
    
    for (auto& System : Systems)
    {        
        System->OnDrawGizmos(GizmosPtr);
    }
    Camera->OnDrawGizmos(GizmosPtr);
    LightPtr->OnDrawGizmos(GizmosPtr);
    Game::Instance->TerrainPtr->OnDrawGizmos(GizmosPtr);
}


void Renderer::CleanUp()
{
    ShaderPtr->CleanUp();
    DepthShaderPtr->CleanUp();
    for (auto& Pass : RenderPasses)
    {
        Pass->CleanUp();
    }
    RenderPasses.clear();
}

shared_ptr<Camera> Renderer::GetCamera() const
{
    return Camera;
}

shared_ptr<Light> Renderer::GetLight()
{
    return LightPtr;
}

std::shared_ptr<Gizmos> Renderer::GetGizmos()
{
    return GizmosPtr;
}

std::shared_ptr<BaseShader> Renderer::GetShaderForCurrentPass() const
{
    if (!CurrentRenderPass)
        return nullptr;
    
    if (CurrentRenderPass->Type == RenderPassType::BasePass)
        return ShaderPtr;

    if (CurrentRenderPass->Type == RenderPassType::ShadowPass)
        return DepthShaderPtr;
    
    if (CurrentRenderPass->Type == RenderPassType::DepthPrePass)
        return ShaderPtr;

    return nullptr;
}

RenderPassType Renderer::GetCurrentRenderPassType() const
{
    return CurrentRenderPass ? CurrentRenderPass->Type : RenderPassType::Invalid;
}


