#pragma once
#include <string>

#include "../Renderer/Camera.h"
#include "GL/glew.h"

namespace TTerrain
{
    class TerrainShader;
}

namespace TTerrain
{
    using namespace std;

    /**
     * Provides access for all thing related to the terrain
     * Can paint a height map via UI
     * Translates this into a triangle blob
     */
    class TerrainManager
    {
    public:
        TerrainManager(const shared_ptr<Camera>& CamPtr);
        ~TerrainManager();

        void Dispatch() const;
        void Render() const;
        void OnDrawGizmos() const;

    private:
        shared_ptr<Camera> CamPtr;
        shared_ptr<TerrainShader> Shader;
        
        unsigned int ComputeProgram;
        GLuint ResultTex;
        GLuint VertexBuffer;
        GLuint VAO;

        void CreateMesh();
        void CreateCompute();
        
        string ComputePath = "./ImportsLib/src/Terrain/Shaders/Terrain.comp";

        float PlaneVertices[18] = {
            0, 0, 0,
            0, 0, 100,
            100, 0, 0,
            0, 0, 100,
            100, 0, 100,
            100, 0, 0,
        };
    };
}
