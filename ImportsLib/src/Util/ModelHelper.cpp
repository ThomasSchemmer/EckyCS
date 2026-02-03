#include "ModelHelper.h"

#include "ShaderHelper.h"
#include "gltf/gltf.hpp"

typedef gltf::Mesh::Primitive::Attribute Attribute;
using Data = std::vector<uint8_t>;

std::vector<ModelData> ModelHelper::LoadGltf(const wchar_t* ResourcePath)
{
    auto Tuple = Util::ShaderHelper::ResourceToData(ResourcePath);
    
    auto File = std::get<0>(Tuple);
    auto Size = std::get<1>(Tuple);
    
    auto Option = gltf::load(File, Size);
    if (!Option.has_value())
        return {};

    gltf::Gltf& Objects = Option.value();
    auto& Meshes = Objects.meshes;

    for (int MeshIndex = 0; MeshIndex < Meshes.size(); ++MeshIndex)
    {
        auto& Mesh = Meshes[MeshIndex];
        for (int PrimitiveIndex = 0; 0 < Mesh.primitives.size(); ++PrimitiveIndex)
        {
            std::vector<short> Triangles;
            auto Vertices = std::vector<glm::vec3>();
            auto Normals = std::vector<glm::vec3>();
            auto UVs = std::vector<glm::vec2>();
            std::vector<ModelData> TargetData; 
            
            auto& Primitive = Mesh.primitives[PrimitiveIndex];
            for (int AttributeIndex = 0; AttributeIndex < Primitive.attributes.size(); ++AttributeIndex)
            {
                auto& Attribute = Primitive.attributes[AttributeIndex];
                bool bUseVec3 = Attribute.id == "POSITION" || Attribute.id == "NORMAL";

                if (bUseVec3)
                {
                    CopyData<glm::vec3>(Objects, MeshIndex, PrimitiveIndex, AttributeIndex, (Attribute.id == "POSITION" ? Vertices : Normals));
                }else
                {
                    CopyData<glm::vec2>(Objects, MeshIndex, PrimitiveIndex, AttributeIndex, UVs);
                }
            }
            
            CopyData<short>(Objects, MeshIndex, PrimitiveIndex, -1, Triangles);

            // interleave for shader
            TargetData.resize(Triangles.size());
            for (int i = 0; i < Triangles.size(); ++i)
            {
                if (Triangles[i] < Vertices.size())    memcpy(&TargetData[i].PositionX, &Vertices[Triangles[i]], sizeof(glm::vec3));
                if (Triangles[i] < UVs.size())         memcpy(&TargetData[i].UVX, &UVs[Triangles[i]], sizeof(glm::vec2));
                if (Triangles[i] < Normals.size())     memcpy(&TargetData[i].NormalX, &Normals[Triangles[i]], sizeof(glm::vec3));
            }
            return TargetData;
        }
    }
    return {};
}
