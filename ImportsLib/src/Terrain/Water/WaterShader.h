#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "../../Renderer/Shaders/Shader.h"
#include "GL/glew.h"
#include "../../Renderer/Passes/RenderPass.h"

class Camera;
class Light;

namespace TTerrain
{
    /**
    * Helper struct to contain all values necessary in the water Shaders
    */
    class WaterShaderSettings
    {
    public:
        RenderPassType RenderPassType;
        glm::vec3 GlobalWorldPos;
        GLuint VertexBuffer;
        glm::ivec2 TexSize;
        glm::vec2 DepthThreshold;
        glm::vec2 ScreenSize;
        
        GLuint DepthTex;

        std::shared_ptr<Camera> Camera;
        std::shared_ptr<Light> Light;
    };
    
    /**
     * Link between the CPU/GPU - handles data passing into vertex/fragment shader
     */
    class WaterShader : public Shader
    {
    public:
        WaterShader() = default;
        void UpdateVars(const WaterShaderSettings& Settings) const;
        void CleanUp() const override;
        void Create() override;
        void Use(RenderPassType Type) override;
        bool SupportsPass(RenderPassType Type) override;
        
    private:
        glm::mat4 Transform;
   
        const wchar_t* VertexShader = L"TERRAIN_WATER_VERTEX_SHADER";
        const wchar_t* FragmentShader = L"TERRAIN_WATER_FRAGMENT_SHADER";

    };
}
