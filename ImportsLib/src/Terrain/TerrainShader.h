#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

class Camera;

namespace TTerrain
{
    using namespace std;
    
    class TerrainShader
    {
    public:
        TerrainShader();
        ~TerrainShader();
        void Use() const;
        void UpdateVars(const shared_ptr<Camera>& Camera) const;

        static glm::vec3 GetMouseWorldPos(const shared_ptr<Camera>& Camera);
    private:
        unsigned int Program;
        glm::mat4 Transform;
   
        string VertexShaderPath = "./ImportsLib/src/Terrain/Shaders/TerrainVertexShader.vert";
        string FragmentShaderPath = "./ImportsLib/src/Terrain/Shaders/TerrainFragmentShader.frag";
    
    };
}
