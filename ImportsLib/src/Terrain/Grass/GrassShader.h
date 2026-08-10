#pragma once

#include <memory>
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>

#include "../../Renderer/Shaders/Shader.h"


class Camera;

namespace TTerrain
{
    class TerrainShaderSettings;

    struct GrassShaderSettings
    {
        GLuint VertexBuffer;
        GLuint PositionBuffer;
        
        std::shared_ptr<Camera> Camera;
    };

    /**
     * Wrapper class to hold the grass shader. Indirect rendered billboard shader
     * that gets its data through the compute shaders
     * */
    class GrassShader : public Shader
    {
    public:
        GrassShader() = default;
        void Create() override;
        void CleanUp() const override;
        bool SupportsPass(RenderPassType Type) override;
        void Use(RenderPassType Type) override;
        void UpdateVars(const GrassShaderSettings& Settings, const TerrainShaderSettings& TerrainSettings) const;

    private:
        GLuint VAO;
        GLuint GrassTex, FlowerTex;
        GLuint FoliageTexArray;
        GLuint GrassProgram;
        glm::mat4 Transform;

        
        const wchar_t* GrassVertexShader = L"TERRAIN_GRASS_VERTEX_SHADER";
        const wchar_t* GrassFragmentShader = L"TERRAIN_GRASS_FRAGMENT_SHADER";
        const wchar_t* GrassTexLocation = L"TERRAIN_GRASS_TEX_LOCATION";
        const wchar_t* FlowerTexLocation = L"TERRAIN_FLOWER_TEX_LOCATION";
    };
}
