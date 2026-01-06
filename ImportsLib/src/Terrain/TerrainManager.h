#pragma once
#define NOGDI
#include <gl\gl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <glm/vec4.hpp>
#include "TerrainData.h"
#include "../EckyCS/Systems/Rendering/SpriteGeometryProvider.h"
#include "Grass/GrassData.h"


namespace EckyCS
{
    class GrassRenderSystem;
}

enum class RenderPassType : uint8_t;
class Gizmos;
class Renderer;
class Light;
class Camera;

namespace TTerrain
{
    class GrassShader;
    using namespace std;
    class TerrainShaderSettings;
    class TerrainShader;

    /**
     * Provides access for all thing related to the terrain
     * Terrain information is divided into chunk, basically a 2d array
     * of @TerrainData, which is then passed into the different shaders
     * There are compute shaders for painting/selecting/creating the terrain
     * as well as display shaders both for the terrain itself and the grass
     * More info on the generation algorithm in @TerrainMesh.comp
     */
    class TerrainManager : public enable_shared_from_this<TerrainManager>
    {
        /** For easy buffer access */
        friend class TerrainData;
        friend class GrassData;
        
    public:
        TerrainManager() = default;
        ~TerrainManager() = default;

        void Init();
        void Render(RenderPassType Type);
        void Update(float Delta);
        void OnDrawGizmos(const shared_ptr<Gizmos>& Gizmos);
        void CleanUp() const;

    private:
        //todo: this is kinda inefficient, better to have one big buffer instead of clustering
        vector<TerrainData> TerrainDatas;
        
        shared_ptr<Camera> CamPtr;
        shared_ptr<Light> LightPtr;
        shared_ptr<TerrainShader> TerrainShader;
        shared_ptr<GrassShader> GrassShader;
        shared_ptr<Renderer> RendererPtr;
        shared_ptr<EckyCS::SpriteGeometryProvider> GeometryProvider;

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
        bool bRenderGrass = true;
        bool bShowWireframe = false;
        
        GLuint ComputeProgramMesh;
        GLuint ComputeProgramPaint;
        GLuint ComputeProgramSelect;
        GLuint ComputeProgramGrass;
        
        GLuint VerticalQuadBuffer;
        GLuint VerticalQuadLengthBuffer;
        GLuint HorizontalQuadBuffer;

        void DispatchCompute();
        void HandleResetting();
        void HandleSelecting() const;
        void HandlePainting();
        void RenderTriangles(RenderPassType Type);
        
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
        
        const wchar_t* ComputeShaderGrass = L"TERRAIN_GRASS_COMPUTE_SHADER";

        static constexpr int QuadIndexCount0 = 15;
        static constexpr int QuadIndexCount1 = 24;

        
        /**
         * These are the four base quadrants everyone has - but they differ in y-position
         * The general order is still the same for all to allow for easy height calculation in-between vertices
         * See shader for more info
         * Quad lookup-table that will be shared with the .comp via ssbo
         */
        static int HorizontalQuadLookup[QuadIndexCount0][QuadIndexCount1];

        /**
         * Contains the vertical slices/walls for each of the different height combinations
         * Each has a variable length and is only filled with -1 for easy access - the shader for more info
         * Quad lookup-table that will be shared with the .comp via ssbo
         */
        static int VerticalQuadLookup[QuadIndexCount0][QuadIndexCount1];
        
        /** Actual length of the vertical buffers */
        static int VerticalQuadLengthLookup[QuadIndexCount0];

    };
}
