#pragma once
#include <memory>
#include "GL/glew.h"
#include <renderdoc/renderdoc_app.h>

#include "Camera.h"
#include "GLFW/glfw3.h"


class Shader;

/**
 * Class enabling displaying objects on the screen
 * Contains shaders to change visuals and helpful debug tools 
 */
class Renderer
{
public:

    Renderer() = default;
    ~Renderer();
    void Init(std::shared_ptr<GLFWwindow>& Window);
    void InitRenderDoc();
    void Update(float Delta);
    void Render();
    void HandleCaptureStart() const;
    void HandleCaptureStop() const;
    shared_ptr<Camera> GetCamera() const;

private:
    Shader* ShaderPtr = nullptr;
    RENDERDOC_API_1_1_2* RDocAPI = nullptr;
    std::shared_ptr<Camera> Camera;
    
    void CreateVertexBuffer();
#ifdef _WIN32
    void LoadRenderDocWindows();
    void UnloadRenderDocWindows() const;
#endif

    char RenderDocPath[42] = "./ImportsLib/ext/renderdoc/renderdoc.dll";
};
