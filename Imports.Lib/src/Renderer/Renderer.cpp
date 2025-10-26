#include "Renderer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include "Shader.h"
#include <windows.h>

#include "../../../ext/imgui/imgui.h"


Renderer::~Renderer()
{
    delete ShaderPtr;
#ifdef _WIN32
    UnloadRenderDocWindows();
#endif
}

void Renderer::Init()
{
    CreateVertexBuffer();
    ShaderPtr = new Shader();
}


void Renderer::InitRenderDoc()
{
#ifdef _WIN32
    LoadRenderDocWindows();
#endif
}

void Renderer::Render() const
{
    ShaderPtr->Use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
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

void Renderer::CreateVertexBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffersARB(1, &VBO);

    glBindVertexArray(VAO);
    glBindBufferARB(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    // specify layout, allow layout 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
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

void Renderer::UnloadRenderDocWindows()
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


