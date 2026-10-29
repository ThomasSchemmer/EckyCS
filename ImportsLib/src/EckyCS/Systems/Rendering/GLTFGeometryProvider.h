#pragma once
#include <memory>
#include <vector>
#include <tuple>

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
            return static_cast<int>(ModelData.size());
        }

        GLsizei GetVertexByteCount() const override
        {
            return sizeof(ModelData) * static_cast<int>(ModelData.size());
        }

        void Load(const wchar_t* Path, const void* Data) override
        {
            const auto* IntData = static_cast<const std::tuple<int, int>*>(Data);
            ModelData = ModelHelper::LoadGltf(Path, std::get<0>(*IntData), std::get<1>(*IntData));
        }

        std::vector<ModelData> ModelData;
    };
}
