#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <glew/include/GL/glew.h>

#include "../Renderer/Passes/RenderPass.h"

class Light;
class Camera;

namespace TTerrain
{
    using namespace std;


    /**
     * Helper struct to contain all values necessary in the Terrain Shaders
     */
    class TerrainShaderSettings
    {
    public:
        RenderPassType RenderPassType;
        glm::vec3 GlobalWorldPos;
        GLuint PositionBuffer, NormalBuffer, HeightBuffer;
        glm::vec3 BrushPos;
        unsigned int BrushSize;
        glm::ivec2 TexSize;
        
        glm::vec3 GrassColor, CliffColor, Tex0Color, Tex1Color, Tex2Color;
        float GrassScale, GrassQuantize;
        bool bShowWireFrame;
        GLuint ShadowMap;
        
        shared_ptr<Camera> Camera;
        shared_ptr<Light> Light;
    };

    /**
     * Link between the CPU/GPU - handles data passing into vertex/fragment shader
     */
    class TerrainShader
    {
    public:
        TerrainShader();
        ~TerrainShader() = default;
        void Use(RenderPassType Type);
        void UpdateVars(const TerrainShaderSettings& Settings) const;
        void CleanUp() const;

        GLuint Program;

    private:
        GLuint DepthProgram;
        GLuint ActiveProgram = 0;
        glm::mat4 Transform;
   
        const wchar_t* VertexShader = L"TERRAIN_VERTEX_SHADER";
        const wchar_t* FragmentShader = L"TERRAIN_FRAGMENT_SHADER";
        const wchar_t* DepthFragmentShader = L"DEPTH_FRAGMENT_SHADER";

    };
}
