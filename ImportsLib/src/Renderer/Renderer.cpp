

#include <filesystem>
#include <fstream>
#include <iostream>
#include "Shader.h"
#define WIN32_LEAN_AND_MEAN
// mask windows byte def, otherwise it overrides std::byte
#define byte win_byte_override
#include <Windows.h>
#undef byte

#include "GL/glew.h"
#define GLFW_INCLUDE_NONE
#include "glfw/include/GLFW/glfw3.h"
#include "Renderer.h"
#include "Utils.h"
#include "../../../ext/imgui/imgui.h"
#include "../EckyCS/ECS.h"
#include "../EckyCS/Systems/Rendering/RenderSystem.h"
#include "../GameService/Game.h"
using namespace EckyCS;

Renderer::~Renderer()
{
    delete ShaderPtr;
#ifdef _WIN32
    UnloadRenderDocWindows();
#endif
}

void Renderer::Init(std::shared_ptr<GLFWwindow>& Window)
{
    ShaderPtr = new Shader();
    Camera = make_shared<class Camera>(Window);

    CreateVertexBuffer();
}


void Renderer::InitRenderDoc()
{
#ifdef _WIN32
    LoadRenderDocWindows();
#endif
}

void Renderer::Update(float Delta)
{
    Camera->Update(Delta);
}

void Renderer::Render()
{
    GL_CHECK_ERROR();
    
    ShaderPtr->Use();
    ShaderPtr->UpdateVars(Camera);

    auto ServicePtr = Game::GetService(GameServiceType::EntityComponentSystem);
    auto Ecs = reinterpret_pointer_cast<ECS>(ServicePtr);
    vector<shared_ptr<System>> Systems;
	if (!Ecs || !Ecs->TryGetSystems<BaseRenderSystem>(OUT Systems))
	    return;

    for (auto& System : Systems)
    {
        auto RenderSystem = dynamic_pointer_cast<BaseRenderSystem>(System);
        if (!RenderSystem)
            continue;
        
        RenderSystem->Render();
    }
    
    for (auto& System : Systems)
    {        
        System->OnDrawGizmos();
    }
    Camera->OnDrawGizmos();
}

void Renderer::HandleCaptureStart() const
{
    if(!RDocAPI)
        return;
    
    ImGui::Begin("RenderDoc");
    if (ImGui::Button("Capture"))
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
}

shared_ptr<Camera> Renderer::GetCamera() const
{
    return Camera;
}

void Renderer::CreateVertexBuffer()
{
    //glGenVertexArrays(1, &VAO);
    //glGenBuffers(1, &VertexBO);
//
    //glBindVertexArray(VAO);
    //glBindBuffer(GL_ARRAY_BUFFER, VertexBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    //// specify layout, 0: vertex pos (vec3), 1: color (vec3), 2: uv (vec2)
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);
    //
    //glGenBuffers(1, &PositionBO);
    //glBindBuffer(GL_ARRAY_BUFFER, PositionBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, Positions, GL_STATIC_DRAW);
    //glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(3);
    //glVertexAttribDivisor(3, 1);  
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


