#pragma once
#define NOGDI
#include <gl\gl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>


class Camera;

namespace TTerrain
{
    using namespace std;
    class TerrainShaderSettings;
    class TerrainShader;

    enum class TerrainComputeMode : uint8_t
    {
        CountTriangles = 0,
        GenerateTriangles = 1,
        GenerateTex = 2,
    };

    enum class TerrainSelectionMode : uint8_t
    {
        Clear = 0,
        Additive = 1,
    };

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

        void DispatchBrush() const;
        void DispatchGenerate();
        void Render();
        void Update(float Delta);
        void OnDrawGizmos();

    private:
        shared_ptr<Camera> CamPtr;
        shared_ptr<TerrainShader> Shader;
        const int Width = 512, Height = 512;
        GLsizei AppendCount = 0;

        glm::vec3 GlobalWorldPos = glm::vec3(0);
        glm::vec2 TexSize = glm::vec2(Width, Height);
        glm::vec3 WorldSize = glm::vec3(100, 25, 100);

        bool bIsSelecting = false;
        bool bWasPressingSelect = false;
        bool bIsRaising = false;
        bool bWasPressingRaise = false;
        
        unsigned int ComputeProgramMesh;
        unsigned int ComputeProgramSelect;
        GLuint ResultTex;
        //GLuint FixedVertexBuffer;
        GLuint VAO;
        GLuint VertexBuffer;
        GLuint NormalBuffer;
        GLuint CountBuffer;
        GLuint SelectionBuffer;

        void HandleInput();
        void HandleToggle(bool* bIsDoing, bool* bWasDoing, GLint Key) const;
        void HandleToggleMouse(bool* bIsDoing, bool* bWasDoing, GLint Key) const;
        
        void CreateMesh();
        void CreateCompute();
        void UpdateComputeVars(GLuint Program) const;
        void Dispatch(GLuint Mode, GLuint Target) const;
        TerrainShaderSettings GetStandardSettings() const;
        
        const wchar_t* ComputeShaderMesh = L"TERRAIN_MESH_COMPUTE_SHADER";
        const wchar_t* ComputeShaderSelect = L"TERRAIN_SELECT_COMPUTE_SHADER";

    };
}
