#pragma once
#include <cassert>
#include <fstream>
#include <iostream>

#include "EckyCS/Components/Base/TransformComponent.h"

namespace EckyCS
{
    struct TransformComponent;
}

using namespace std;
using namespace EckyCS;
class VoxReader
{
public:

    static int ReadChunk(ifstream& TargetFile, vector<vector<TransformComponent>>& Voxels)
    {
        char ChunkType[4];
        int BytesInChunk;
        int BytesInChildren;
        TargetFile.read(&ChunkType[0], 4);
        TargetFile.read(reinterpret_cast<char*>(&BytesInChunk), 4);
        TargetFile.read(reinterpret_cast<char*>(&BytesInChildren), 4);

        int SelfReadCount = 3 * sizeof(int);
        if (string_view(ChunkType, 4) == "SIZE")
        {
            return ReadChunkSize(TargetFile, Voxels) + SelfReadCount;
        }
        if (string_view(ChunkType, 4) == "XYZI")
        {
            return ReadChunkData(TargetFile, Voxels) + SelfReadCount;
        }
        if (string_view(ChunkType, 4) == "RGBA")
        {
            return ReadChunkColor(TargetFile, Voxels) + SelfReadCount;
        }
        return SelfReadCount; 
    }

    static int ReadChunkColor(ifstream& TargetFile, vector<vector<TransformComponent>>& Voxels)
    {
        cout << "ReadChunkColor" << "\n";
        return 0;
    }
    
    static int ReadChunkSize(ifstream& TargetFile, vector<vector<TransformComponent>>& Voxels)
    {
        // we dont actually have to do much here i think?
        int SizeX, SizeY, SizeZ;
        TargetFile.read(reinterpret_cast<char*>(&SizeX), 4);
        TargetFile.read(reinterpret_cast<char*>(&SizeY), 4);
        TargetFile.read(reinterpret_cast<char*>(&SizeZ), 4);

        int SelfReadCount = 3 * sizeof(int);
        return SelfReadCount;
    }

    static int ReadChunkData(ifstream& TargetFile, vector<vector<TransformComponent>>& Voxels)
    {
        int NumVoxels;
        TargetFile.read(reinterpret_cast<char*>(&NumVoxels), sizeof(int));

        int NumVoxelsByte = NumVoxels * sizeof(int);
        vector<TransformComponent> Data(NumVoxels);
        vector<byte> DataByte(NumVoxelsByte);
        TargetFile.read(reinterpret_cast<char*>(DataByte.data()), NumVoxelsByte);
        for (int i = 0; i < NumVoxels; ++i) {
            int* vPtr = reinterpret_cast<int*>(&DataByte[i * sizeof(int)]);
            Data[i].PosX = static_cast<float>(*vPtr & 0xFF);          
            Data[i].PosY = -static_cast<float>((*vPtr >> 8) & 0xFF);   
            Data[i].PosZ = static_cast<float>((*vPtr >> 16) & 0xFF);  
            // ignore 4th byte, color value
        }
        Voxels.push_back(Data);
        return 4 * sizeof(int) + NumVoxelsByte;
    }

    static void ReadChunkMain(ifstream& TargetFile, vector<vector<TransformComponent>>& Voxels)
    {
        char ChunkType[4];
        int BytesInChunk;
        int BytesInChildren;
        TargetFile.read(&ChunkType[0], 4);
        TargetFile.read(reinterpret_cast<char*>(&BytesInChunk), 4);
        TargetFile.read(reinterpret_cast<char*>(&BytesInChildren), 4);

        assert(BytesInChunk == 0);
        assert(BytesInChildren != 0);
        
        int ByteCount = 0;
        while (ByteCount < BytesInChildren)
        {
            ByteCount += ReadChunk(TargetFile, Voxels);
        }
    }

    static vector<vector<TransformComponent>> LoadVoxFile()
    {
        vector<vector<TransformComponent>> Voxels;
        try {
            ifstream TargetFile;
            TargetFile.open(VoxFilePath, ios::binary);  
            
            char Name[4];
            int Version;
            TargetFile.read(&Name[0], 4);
            TargetFile.read(reinterpret_cast<char*>(&Version), 4);
            ReadChunkMain(TargetFile, Voxels);

            TargetFile.close();
        } catch (ifstream::failure& e) {
            cerr << "ERROR::VOX::FILE_MISSING: " << e.what() << "\n";
        }

        return Voxels;
    }
    
    inline static string VoxFilePath = "C:/Users/Thoma/Downloads/pieta/pieta.vox";
    //inline static string VoxFilePath = "C:/Users/Thoma/Downloads/pieta/DragonRoom.vox";
};
