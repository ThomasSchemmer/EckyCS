#pragma once
#include "gltf/gltf.hpp"

namespace gltf
{
    struct Gltf;
}

struct ModelData
{
public:
    float PositionX, PositionY, PositionZ;
    float UVX, UVY;;
    float NormalX, NormalY, NormalZ;
};

class ModelHelper
{
public:
    static std::vector<ModelData> LoadGltf(const wchar_t* ResourcePath);

    template<typename T>
    static void CopyData(const gltf::Gltf& Object, int MeshIndex, int PrimitiveIndex, int AttributeIndex, std::vector<T>& TargetVector)
    {
        // sadly we need to redefine all of this to avoid passing in even more references
        auto& Buffers = Object.buffers;
        auto& BufferViews = Object.bufferViews;
        auto& Meshes = Object.meshes;
        auto& Accessors = Object.accessors;

        const gltf::Mesh& Mesh = Meshes[MeshIndex];
        const gltf::Mesh::Primitive& Primitive = Mesh.primitives[PrimitiveIndex];
        
        const size_t& AccessorIndex = AttributeIndex >= 0 ?
            Primitive.attributes[AttributeIndex].accessor : Primitive.indices.value();
        auto& Accessor = Accessors[AccessorIndex];

        size_t BufferViewIndex = Accessor.bufferView.value();
        auto& BufferView = BufferViews[BufferViewIndex];
        size_t BufferIndex = BufferView.buffer;

        auto& Data = Buffers[BufferIndex].data;
        auto Offset = BufferView.byteOffset;
        auto Length = BufferView.byteLength;

        size_t TargetCount = Length / sizeof(T);
        if (TargetCount != Accessor.count)
            return;

        TargetVector.resize(TargetCount);
        memcpy(TargetVector.data(), Data.data() + Offset, Length);
    }


};
