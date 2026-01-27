#include "TerrainHelper.h"

#include "TerrainData.h"
#include "../GameService/Game.h"
#include "GLFW/glfw3.h"
#include "glm/common.hpp"
#include "glm/vec2.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "imgui/imgui.h"
#include "../Renderer/Renderer.h"

namespace TTerrain
{
    using namespace glm;
    using namespace std;
    
    std::vector<std::tuple<unsigned int, GLsync>> TerrainHelper::Fences;
    std::vector<vec4> TerrainHelper::Vertices;
    std::vector<int> TerrainHelper::Offsets;
    std::vector<unsigned int> TerrainHelper::HeightDatas;

    void TerrainHelper::DeleteFence(int i)
    {
        const auto& Tuple = Fences[i];
        GLenum result = glClientWaitSync(
            get<1>(Tuple),
            0,              
            0               
        );

        if (result == GL_ALREADY_SIGNALED ||
            result == GL_CONDITION_SATISFIED)
        {
            glDeleteSync(get<1>(Tuple));
            Fences.erase(Fences.begin() + i);
        }
    }

    void TerrainHelper::GetQuadIndices(vec3 WorldPos, vec2 WorldSize, int& QuadIndex, ivec2& QuadXY)
    {
        auto TexSize = ivec2(TerrainData::TexSize);
        
        // use the world pos to lookup which quad its supposed to reference
        vec2 Scale = vec2((float)TerrainData::TexSize / WorldSize.x, (float)TerrainData::TexSize / WorldSize.y);
        auto ScaledWorldPos = vec3(WorldPos.x * Scale.x, 0, WorldPos.z * Scale.y);
        QuadIndex = GetQuadIndexFor(ScaledWorldPos, ivec2(0));
        QuadXY = ivec2(QuadIndex % TexSize.x, QuadIndex / TexSize.y);
    }

    int TerrainHelper::GetQuadIndexFor(vec3 id, ivec2 Offset){
        auto TexSize = vec2(TerrainData::TexSize);
        float X = std::floor(id.z + (float)Offset.y);
        float Z = std::floor(id.x + (float)Offset.x);
        float ClampedID =
            clamp(X, 0.0f, TexSize.y - 1) * TexSize.x +
            clamp(Z, 0.0f, TexSize.x - 1);
        return (int)ClampedID;
    }

    float TerrainHelper::GetSideOfTriangle(int IndexInQuad, int StartVertex, vec3 WorldPos, const vector<vec4>& Vertices)
    {
        vec3 Min03 = IndexInQuad == 0 ? Vertices[StartVertex +  0] : Vertices[StartVertex + 18];
        vec3 Max03 = IndexInQuad == 0 ? Vertices[StartVertex +  5] : Vertices[StartVertex + 23];
        vec3 Min12 = IndexInQuad == 1 ? Vertices[StartVertex +  8] : Vertices[StartVertex + 14];
        vec3 Max12 = IndexInQuad == 1 ? Vertices[StartVertex + 11] : Vertices[StartVertex + 17];
        vec3 Min = IndexInQuad == 0 || IndexInQuad == 3 ? Min03 : Min12;
        vec3 Max = IndexInQuad == 0 || IndexInQuad == 3 ? Max03 : Max12;

        // calculate the divider line
        vec2 A = vec2(Max.x, Min.z);
        vec2 B = vec2(Min.x, Max.z);
        vec2 AB = B - A;
        AB = vec2(AB.y, -AB.x);
        vec2 APos = vec2(WorldPos.x, WorldPos.z) - A;
        float d = dot(AB, APos);
        // now we can check which side we are on
        float Side = sign(d);
        return Side;
    }

    vec3 TerrainHelper::GetBaricentricCoordinates(vec3 WorldPos, const vec3& VertA, const vec3& VertB, const vec3& VertC)
    {
        auto VecAB = VertB - VertA;
        auto VecAC = VertC - VertA;
        auto VecAP = WorldPos - VertA;
        auto d00 = dot(VecAB, VecAB);
        auto d01 = dot(VecAB, VecAC);
        auto d11 = dot(VecAC, VecAC);
        auto d20 = dot(VecAP, VecAB);
        auto d21 = dot(VecAP, VecAC);
        float Denom = d00 * d11 - d01 * d01;
        float beta = (d11 * d20 - d01 * d21) / Denom;
        float gamma = (d00 * d21 - d01 * d20) / Denom;
        float alpha = 1 - beta - gamma;
        return {alpha, beta, gamma};
    }

    float TerrainHelper::GetHeightFromWorldPos(vec3 WorldPos, vec2 WorldSize)
    {
        auto ClosestVertices = GetClosestVertices(WorldPos, WorldSize);
        auto BariCoords = GetBaricentricCoordinates(WorldPos, ClosestVertices[0], ClosestVertices[1], ClosestVertices[2]);
        // and now we can use that to interpolate the height
        return BariCoords.x * ClosestVertices[0].y + BariCoords.y * ClosestVertices[1].y + BariCoords.z * ClosestVertices[2].y;
    }

    unsigned int TerrainHelper::GetClosestHeightData(vec3 WorldPos, vec2 WorldSize)
    {
        if (HeightDatas.empty())
            return 0;
        
        int QuadIndex;
        ivec2 QuadXY;
        GetQuadIndices(WorldPos, WorldSize, QuadIndex, QuadXY);
        return HeightDatas[QuadIndex];
    }

    vector<vec3> TerrainHelper::GetClosestVertices(vec3 WorldPos, vec2 WorldSize)
    {
        auto TexSize = ivec2(TerrainData::TexSize);
        int QuadIndex;
        ivec2 QuadXY;
        GetQuadIndices(WorldPos, WorldSize, QuadIndex, QuadXY);
        
        // since all terrain quadrants have the standard quad at their base, we need to check for offset
        vec2 WorldSizePerQuad = vec2(WorldSize.x, WorldSize.y) / vec2(TexSize);
        vec2 WorldPosOfQuad;
        WorldPosOfQuad.x = WorldSizePerQuad.x * (float)QuadXY.x;
        WorldPosOfQuad.y = WorldSizePerQuad.y * (float)QuadXY.y;

        vec2 WorldPosInQuad = vec2(WorldPos.x, WorldPos.z) - WorldPosOfQuad;
        vec2 WorldPosInQuadPerc = WorldPosInQuad / WorldSizePerQuad;
        
        // for more info on the layout see @TerrainMesh.comp
        // we try to find the quad first, then need to check which triangle of that quad we are in
        int StartVertex = QuadIndex == 0 ? 0 : Offsets[QuadIndex - 1];
        int IndexInQuad = (WorldPosInQuadPerc.x > .5 ? 1 : 0) + (WorldPosInQuadPerc.y > .5 ? 2 : 0);

        // unfortunately the order of the triangle in a quadpart is one of two, depending on triangle index
        // so offset from the start depending on which one we are looking at (each quadpart has 2 triangles aka 6 vertices)
        // for that we need to check on which side of the divider line we are on
        float Side = GetSideOfTriangle(IndexInQuad, StartVertex, WorldPos, Vertices);

        // but we still need to invert the order for quads 1 and 2, as they are flipped!
        int TriangleStart = StartVertex + IndexInQuad * 6;
        int SideOffset = (IndexInQuad == 1 || IndexInQuad == 2) ? 3 : 0;
        SideOffset = (Side > 0 ? 3 - SideOffset : SideOffset);
        TriangleStart += SideOffset;
        
        // get the actual triangle positions
        auto VertA = Vertices[TriangleStart + 0];
        auto VertB = Vertices[TriangleStart + 1];
        auto VertC = Vertices[TriangleStart + 2];

        vector<vec3> ClosestVertices = {VertA, VertB, VertC};
        return ClosestVertices;
    }

    void TerrainHelper::UpdateData(TerrainManager* TerrainManager)
    {
        // we always expect the first entry to be the actual queried!
        if (glClientWaitSync(get<1>(Fences[0]), 0, 0) != GL_ALREADY_SIGNALED)
            return;
        
        GPU_PROFILE(GameImports::Game::GetGpuFrame(), "ShaderHelper::UpdateData", legit::Colors::wisteria);
        //TODO:: make chunks!
        const auto& Data = TerrainManager->GetData(0);

        auto TexSize = uvec2(TerrainData::TexSize);
        auto TotalSize = TexSize.x * TexSize.y;
        //TODO: get actual size!
        auto HeightBufferByteSize = TotalSize;
        auto VertexBufferByteSize = TotalSize * 24;
        Vertices.resize(VertexBufferByteSize);
        Offsets.resize(HeightBufferByteSize);
        HeightDatas.resize(HeightBufferByteSize);
        
        memcpy(Vertices.data(), Data.MappedVertexPtr, VertexBufferByteSize * sizeof(vec4));
        memcpy(Offsets.data(), Data.MappedVertexOffsetPtr, HeightBufferByteSize * sizeof(unsigned int));
        memcpy(HeightDatas.data(), Data.MappedHeightPtr, HeightBufferByteSize * sizeof(unsigned int));
    }

    void TerrainHelper::Update(TerrainManager* TerrainManager)
    {
        auto CurrentFrame = GameImports::Game::FrameCounter;
        // get the oldest, valid frame
        for (int i = Fences.size() - 1; i >= 0; i--)
        {
            const auto& Tuple = Fences[i];
            if (get<0>(Tuple) > CurrentFrame - 2)
                continue;

            // there are some older ones, so we can ignore the requests in between
            if (i != 0)
            {
                DeleteFence(i);
                continue;
            }
            
            UpdateData(TerrainManager);
            DeleteFence(i);
        }
        
        if (!TerrainManager->IsDirty())
            return;

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        auto Fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        Fences.emplace_back(GameImports::Game::FrameCounter, Fence);
    }
}
