#pragma once
#include <memory>
#include <GL/glew.h>
#include <renderdoc/renderdoc_app.h>

#include "Camera.h"
#include "GLFW/glfw3.h"


namespace TTerrain
{
    class TerrainManager;
}

class BaseShader;

/**
 * Class enabling displaying objects on the screen
 * Contains shaders to change visuals and helpful debug tools 
 */
class Renderer
{
public:

    Renderer() = default;
    ~Renderer();
    void Init(GLFWwindow* Window);
    void InitRenderDoc();
    void Update(float Delta);
    void Render();
    void HandleCaptureStart() const;
    void HandleCaptureStop() const;
    void CleanUp() const;
    shared_ptr<Camera> GetCamera() const;

private:
    RENDERDOC_API_1_1_2* RDocAPI = nullptr;
    std::shared_ptr<Camera> Camera;
    std::shared_ptr<TTerrain::TerrainManager> Terrain;
    std::shared_ptr<BaseShader> ShaderPtr;
    
    void CreateVertexBuffer();
#ifdef _WIN32
    void LoadRenderDocWindows();
    void UnloadRenderDocWindows() const;
#endif

    char RenderDocPath[42] = "./ImportsLib/ext/renderdoc/renderdoc.dll";
};
