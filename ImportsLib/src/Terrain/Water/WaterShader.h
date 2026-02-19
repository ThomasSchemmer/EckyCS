#pragma once
#include <memory>
#include <glm/glm.hpp>
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
        
        GLuint DepthTex;

        std::shared_ptr<Camera> Camera;
        std::shared_ptr<Light> Light;
    };
    
    /**
     * Link between the CPU/GPU - handles data passing into vertex/fragment shader
     */
    class WaterShader
    {
    public:
        WaterShader();
        ~WaterShader() = default;
        void Use(RenderPassType Type);
        void UpdateVars(const WaterShaderSettings& Settings) const;
        void CleanUp() const;
        
        GLuint Program = 0;

    private:
        glm::mat4 Transform;
   
        const wchar_t* VertexShader = L"TERRAIN_WATER_VERTEX_SHADER";
        const wchar_t* FragmentShader = L"TERRAIN_WATER_FRAGMENT_SHADER";

    };
}
