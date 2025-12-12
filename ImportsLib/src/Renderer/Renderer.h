#pragma once
#include <memory>
#include <glew/include/GL/glew.h>
#include <renderdoc/renderdoc_app.h>

#include "Camera.h"
#include "Gizmos.h"
#include "Light.h"
#include "GLFW/glfw3.h"
#include "Passes/RenderPass.h"


class DepthShader;

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
    ~Renderer() = default;
    void Init(GLFWwindow* Window);
    void InitRenderDoc();
    void Update(float Delta) const;
    void Render();
    void HandleCaptureStart() const;
    void HandleCaptureStop() const;
    void CleanUp();
    std::shared_ptr<Camera> GetCamera() const;
    std::shared_ptr<Light> GetLight();
    std::shared_ptr<Gizmos> GetGizmos();
    RenderPassType GetCurrentRenderPassType() const;

    template<typename T>
    requires std::is_base_of_v<RenderPass, T>
    T* GetRenderPass()
    {
        for (auto& Pass : RenderPasses)
        {
            auto TPass = dynamic_cast<T*>(Pass.get());
            if (!TPass)
                continue;

            return TPass;
        }
        return nullptr;
    }

private:
    RENDERDOC_API_1_1_2* RDocAPI = nullptr;
    std::shared_ptr<Camera> Camera;
    std::shared_ptr<Light> LightPtr;
    std::shared_ptr<TTerrain::TerrainManager> Terrain;
    std::shared_ptr<BaseShader> ShaderPtr;
    std::shared_ptr<DepthShader> DepthShaderPtr;
    std::shared_ptr<Gizmos> GizmosPtr;
    std::vector<std::shared_ptr<RenderPass>> RenderPasses;
    std::shared_ptr<RenderPass> CurrentRenderPass;

    void InitRenderPasses(GLFWwindow* Window);
    
#ifdef _WIN32
    void LoadRenderDocWindows();
    void UnloadRenderDocWindows() const;
#endif

    char RenderDocPath[42] = "./ImportsLib/ext/renderdoc/renderdoc.dll";
};
