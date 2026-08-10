#pragma once
#include <memory>
#include <glew/include/GL/glew.h>

#include "Camera.h"
#include "Gizmos.h"
#include "Light.h"
#include "GLFW/glfw3.h"
#include "Passes/RenderPass.h"
#include "../EckyCS/Systems/System.h"

namespace EckyCS
{
    class System;
}

namespace PostProcessing
{
    class PostProcessingManager;
}

class BasePass;
class PixelShader;
class DepthShader;
class BaseShader;

namespace Scene
{
    class SceneManager;
}

namespace TTerrain
{
    class TerrainManager;
}


/**
 * Class enabling displaying objects on the screen
 * Contains shaders to change visuals and helpful debug tools 
 */
class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    /** Create all sub-renderers and shaders*/
    void Init(GLFWwindow* Window);
    
    /** Update Light and Camera, position etc*/
    void Update(float Delta) const;

    /** Render all passes, for all things from ECS, PostProcessing and Terrain*/
    void Render();

    /** Resets viewport size and FBO to standard settings*/
    void Reset();

    /** Destroys all render related objects*/
    void CleanUp();
    
    std::shared_ptr<Camera> GetCamera() const;
    std::shared_ptr<Light> GetLight();
    std::shared_ptr<Gizmos> GetGizmos();
    std::shared_ptr<BaseShader> GetShaderForCurrentPass() const;
    RenderPassType GetCurrentRenderPassType() const;

    GLuint HasFrameBuffer(FrameBufferTarget Target) const;
    void SetFrameBuffer(FrameBufferTarget Target, GLuint FBO);

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
    std::shared_ptr<Camera> Camera;
    std::shared_ptr<Light> LightPtr;
    
    std::shared_ptr<BaseShader> ShaderPtr;
    std::shared_ptr<DepthShader> DepthShaderPtr;
    
    std::shared_ptr<Gizmos> GizmosPtr;
    std::shared_ptr<Scene::SceneManager> SceneManagerPtr;
	std::shared_ptr<PostProcessing::PostProcessingManager> PostProcessingPtr;
    std::vector<std::shared_ptr<RenderPass>> RenderPasses;
    std::map<FrameBufferTarget, GLuint> FrameBufferObjects;
    std::shared_ptr<RenderPass> CurrentRenderPass;

    void InitRenderPasses(GLFWwindow* Window);
    void RenderECS(const shared_ptr<RenderPass>& Pass, const vector<shared_ptr<EckyCS::System>>& Systems) const;
};
