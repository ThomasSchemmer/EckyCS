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
        GLuint ResultTex;
        GLuint VertexBuffer, NormalBuffer, SelectionBuffer;
        glm::vec3 BrushPos;
        glm::ivec2 TexSize;
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
   
        string VertexShaderPath = "./ImportsLib/src/Terrain/Shaders/TerrainVertexShader.vert";
        string FragmentShaderPath = "./ImportsLib/src/Terrain/Shaders/TerrainFragmentShader.frag";
    
    };
}
