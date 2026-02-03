#pragma once
#include <memory>
#include <vector>

#include "GeometryProvider.h"
#include "../../../Util/ModelHelper.h"
#include "glm/vec3.hpp"

namespace EckyCS
{
    class GLTFGeometryProvider : public GeometryProvider
    {
    public:
        void* GetVertexArray() override
        {
            return ModelData.data();
        }

        GLsizei GetVertexCount() const override
        {
            return ModelData.size();
        }

        GLsizei GetVertexByteCount() const override
        {
            return sizeof(ModelData) * ModelData.size();
        }

        void Load(const wchar_t* Path) override
        {
            ModelData = ModelHelper::LoadGltf(Path);
        }

        std::vector<ModelData> ModelData;
    };
}
