#pragma once
#define NOGDI
#include <gl\gl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <glm/vec4.hpp>
#include "TerrainData.h"


class Renderer;
class Light;
class Camera;

namespace TTerrain
{
    using namespace std;
    class TerrainShaderSettings;
    class TerrainShader;

    /**
     * Provides access for all thing related to the terrain
     * Can paint a height map via UI
     * Translates this into a triangle blob
     */
    class TerrainManager
    {
    public:
        TerrainManager();
        ~TerrainManager() = default;

        void Render();
        void Update(float Delta);
        void OnDrawGizmos();
        void CleanUp() const;

    private:
        //todo: this is kinda inefficient, better to have one big buffer instead of clustering
        vector<TerrainData> Datas;
        shared_ptr<Camera> CamPtr;
        shared_ptr<Light> LightPtr;
        shared_ptr<TerrainShader> Shader;
        shared_ptr<Renderer> RendererPtr;

        bool bIsEditing = false;
        bool bIsSelecting = false;
        bool bWasPressingSelect = false;
        bool bIsRaising = false;
        bool bWasPressingRaise = false;
        bool bIsResetting = false;
        glm::vec2 BrushStartScreenPos;
        glm::vec3 SelectStartWorldPos;
        glm::vec3 RaiseStartWorldPos;
        int BrushStrength = 1;
        int BrushSize = 1;

        glm::vec3 GrassColor = glm::vec3(0.21, 0.94, 0.28);
        glm::vec3 DirtColor = glm::vec3(0.87, 0.75, 0.63);
        float GrassScale = 0.015f, GrassQuantize = 7;
        
        GLuint ComputeProgramMesh;
        GLuint ComputeProgramPaint;
        GLuint ComputeProgramSelect;

        void DispatchCompute();
        void HandleResetting();
        void HandleSelecting() const;
        void HandlePainting();
        void RenderTriangles() const;
        
        void HandleInput();
        void HandleToggle(bool* bIsDoing, bool* bWasDoing, GLint Key) const;
        void HandleToggleMouse(bool* bIsDoing, bool* bWasDoing, GLint Key) const;
        int GetBrushDirection() const;

        void CreateTerrainAt(glm::vec3 WorldPos);
        void CreateCompute();
        
        void SaveData() const;
        void LoadData();
        TerrainShaderSettings GetStandardSettings() const;
        void UpdateComputeVars(GLuint Program) const;
        GLsizei GetTotalAppendCount() const; 
        
        const wchar_t* ComputeShaderMesh = L"TERRAIN_MESH_COMPUTE_SHADER";
        const wchar_t* ComputeShaderPaint = L"TERRAIN_PAINT_COMPUTE_SHADER";
        const wchar_t* ComputeShaderSelect = L"TERRAIN_SELECT_COMPUTE_SHADER";

    };
}
