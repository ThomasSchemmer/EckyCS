

#include <filesystem>
#include <fstream>
#include <iostream>
#include "BaseShader.h"
#define WIN32_LEAN_AND_MEAN
// mask windows byte def, otherwise it overrides std::byte
#define byte win_byte_override
#include <Windows.h>
#undef byte

#include <glew/include/GL/glew.h>
#define GLFW_INCLUDE_NONE
#include "glfw/include/GLFW/glfw3.h"
#include "Renderer.h"
#include "../../../ext/imgui/imgui.h"
#include "../EckyCS/ECS.h"
#include "../EckyCS/Systems/Rendering/EntityRenderSystem.h"
#include "../GameService/Game.h"
#include "../Terrain/TerrainManager.h"
#include "Passes/BasePass.h"
#include "Passes/ShadowPass.h"
#include "Util/Utils.h"
using namespace EckyCS;


void Renderer::Init(GLFWwindow* Window)
{
    ShaderPtr = make_shared<BaseShader>();
    Camera = make_shared<class Camera>(Window);
    LightPtr = make_shared<Light>(
        glm::vec3(10),
        glm::vec3(glm::radians(125.0f), glm::radians(140.0f), 0),
        glm::vec3(1),
        Window,
        Camera
    );
    Terrain = make_shared<TTerrain::TerrainManager>();

    InitRenderPasses(Window);
}


void Renderer::InitRenderDoc()
{
#ifdef _WIN32
    LoadRenderDocWindows();
#endif
}

void Renderer::InitRenderPasses(GLFWwindow* Window)
{
    auto SPass = make_shared<ShadowPass>();
    SPass->Create(Window);
    auto BPass = make_shared<BasePass>();
    BPass->Create(Window);
    RenderPasses.emplace_back(SPass);
    RenderPasses.emplace_back(BPass);
}

void Renderer::Update(float Delta) const
{
    Camera->Update(Delta);
    LightPtr->Update(Delta);
    Terrain->Update(Delta);
}

void Renderer::Render()
{
    GL_CHECK_ERROR();

    auto ServicePtr = Game::GetService(GameServiceType::EntityComponentSystem);
    auto Ecs = reinterpret_pointer_cast<ECS>(ServicePtr);
    vector<shared_ptr<System>> Systems;
    Ecs->TryGetSystems<BaseRenderSystem>(OUT Systems);
    
    for (auto& Pass : RenderPasses)
    {
        Pass->Use();
        CurrentRenderPass = Pass;
        
        Terrain->Render();
    
        ShaderPtr->Use();
        ShaderPtr->UpdateVars(Camera, LightPtr);

        for (auto& System : Systems)
        {
            auto RenderSystem = dynamic_pointer_cast<BaseRenderSystem>(System);
            if (!RenderSystem)
                continue;
        
            RenderSystem->Render();
        }
        
        CurrentRenderPass = nullptr;
        Pass->UnUse();
    }
    
    for (auto& System : Systems)
    {        
        System->OnDrawGizmos();
    }
    Camera->OnDrawGizmos();
    Terrain->OnDrawGizmos();
    LightPtr->OnDrawGizmos();
}

void Renderer::HandleCaptureStart() const
{
    if(!RDocAPI)
        return;
    
    ImGui::Begin("RenderDoc");
    if (ImGui::Button("Capture") || Camera->GetKey(GLFW_KEY_F11) == GLFW_PRESS)
    {
        RDocAPI->StartFrameCapture(nullptr, nullptr);
    }
    ImGui::End();
}

void Renderer::HandleCaptureStop() const
{
    if (!RDocAPI || !RDocAPI->IsFrameCapturing())
        return;

    RDocAPI->EndFrameCapture(nullptr, nullptr);

    char pathBuffer[4096];
    auto Num = RDocAPI->GetNumCaptures();
    RDocAPI->GetCapture(Num - 1, pathBuffer, nullptr, nullptr);
    
    RDocAPI->LaunchReplayUI(1, pathBuffer);
}

void Renderer::CleanUp()
{
    Terrain->CleanUp();
    ShaderPtr->CleanUp();
    for (auto& Pass : RenderPasses)
    {
        Pass->CleanUp();
    }
    RenderPasses.clear();
#ifdef _WIN32
    UnloadRenderDocWindows();
#endif
}

shared_ptr<Camera> Renderer::GetCamera() const
{
    return Camera;
}

shared_ptr<Light> Renderer::GetLight()
{
    return LightPtr;
}

RenderPassType Renderer::GetCurrentRenderPassType() const
{
    return CurrentRenderPass ? CurrentRenderPass->Type : RenderPassType::Invalid;
}

#ifdef _WIN32
void Renderer::LoadRenderDocWindows()
{
    if(HMODULE RDC = LoadLibraryA(RenderDocPath))
    {
        auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(RDC, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&RDocAPI);
        if (ret != 1)
        {
            throw std::runtime_error("ERROR::RENDERER::FAILED_LOADING_RENDERDOC\n");
        }
    }
}

void Renderer::UnloadRenderDocWindows() const
{
    if (RDocAPI == nullptr)
        return;

    if (HMODULE RDC = GetModuleHandleA(RenderDocPath)) {
        RDocAPI->Shutdown();
        if (FreeLibrary(RDC)) {
            RDC = nullptr;
        } 
    }
}
#endif


