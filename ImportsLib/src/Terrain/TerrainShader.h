#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <glew/include/GL/glew.h>

class Camera;

namespace TTerrain
{
    using namespace std;

    
    class TerrainShaderSettings
    {
    public:
        glm::vec3 GlobalWorldPos;
        GLuint VertexBuffer, NormalBuffer, HeightBuffer, SelectionBuffer;
        glm::vec3 BrushPos;
        unsigned int BrushSize;
        glm::ivec2 TexSize, WorldSize;
        
        glm::vec3 GrassColor, DirtColor;
        float GrassScale, GrassQuantize;
    };
    
    class TerrainShader
    {
    public:
        TerrainShader(int Width, int Height);
        ~TerrainShader();
        void Use() const;
        void UpdateVars(const shared_ptr<Camera>& Camera, const TerrainShaderSettings& Settings) const;

    private:
        unsigned int Program;
        glm::mat4 Transform;
        int Width, Height;
   
        const wchar_t* VertexShader = L"TERRAIN_VERTEX_SHADER";
        const wchar_t* FragmentShader = L"TERRAIN_FRAGMENT_SHADER";
    
    };
}
