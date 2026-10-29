#pragma once

#include <glew/include/GL/glew.h>
#include "../../Components/ComponentGroupIdentifier.h"
#include "../../Components/Base/TransformComponent.h"
#include "../../Entities/Entity.h"
#include "../../Util/TypeInfo.h"

namespace EckyCS
{
    class GeometryProvider;
}

namespace EckyCS
{

    template <typename T>
    concept HasModelDefinition = requires
    {
        { T::Model.Name } -> std::convertible_to<std::wstring_view>;
        { T::Model.MeshIndex } -> std::convertible_to<std::uint32_t>;
        { T::Model.PrimitiveIndex } -> std::convertible_to<std::uint32_t>;
    };

    // Forces any entity to be rendered to inherit from renderable instead
    template <typename T>
    concept IsRenderable = requires
    {
        typename T::Renderable;
    };
    
    struct ModelDefinition
    {
        const wchar_t* Name;
        std::uint32_t MeshIndex;
        std::uint32_t PrimitiveIndex;
    };
    
    /**
    * Abstract helper class to manage data flow from the CPU to GPU
     * RenderData that has to explicitly copied to the GPU every frame
     * Used to integrate Entities into the Rendering Pipeline
     * Copies all Components specified by the targeted Entity type, but
     * has to use Ptr/lookup for non-standard ones
     * Also defines what we want to render by enforcing the inheriting data
     * to include its model data
     */
    class RenderData
    {
    public:
        using Renderable = void;
        
        size_t Count;

        RenderData() = default;
        virtual ~RenderData() = default;

        template <typename RenderDataType> requires
            is_base_of_v<RenderData, RenderDataType> &&
            HasModelDefinition<RenderDataType>
        void Create(size_t count, const shared_ptr<GeometryProvider>& Provider)
        {
            std::tuple<int, int> Data = std::make_tuple(
                RenderDataType::Model.MeshIndex, RenderDataType::Model.PrimitiveIndex);
            CreateInternal(RenderDataType::Model.Name, &Data, count, Provider);
        }
        
        void Render() const;

        /** Copies the Transforms, IDs, and any additionally defined compoments from CPU to GPU */
        template <typename... Components>
        requires AllContainedIn<tuple<TransformComponent>, tuple<Components...>>
        void UpdateBuffers(ComponentGroupIdentifier GroupID, size_t InCount, View<Components...> Data)
        {
            //todo: make sure we actually transport this only onc per frame and reus it.
            // also make faster?
            auto IDPtr = GetID(Data).data();
            auto TransformPtr = Get<TransformComponent>(Data).data();
            
            glBindBuffer(GL_ARRAY_BUFFER, OffsetBuffer);
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                InCount * sizeof(TransformComponent),
                TransformPtr
            );            
            glBindBuffer(GL_ARRAY_BUFFER, IDBuffer);
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                InCount * sizeof(int),
                IDPtr
            );

            // since we can't override a templated function in subclasses
            // we need to store lookup info for each wanted type, which is registered
            // by the @RenderData each update
            for (auto Pair : ParamLookup)
            {
                auto GlIndex = get<0>(Pair.second);
                auto Size = get<1>(Pair.second);
                auto Index = get<2>(Pair.second);
                auto TargetPtr = TupleAt(Data, Index);

                glBindBuffer(GL_ARRAY_BUFFER, GlIndex);
                glBufferSubData(
                    GL_ARRAY_BUFFER,
                    0,
                    InCount * Size,
                    TargetPtr
                );
            }
        }
        
    protected:
        virtual void CreateInternal(const wchar_t* Path, void* Data, 
            size_t count, const shared_ptr<GeometryProvider>& Provider);

        
        GLuint VAO;

        shared_ptr<GeometryProvider> DataProvider;
        
        // stores GL-ID, size of component and index of type in View
        map<TypeInfo, tuple<int, size_t, int>> ParamLookup;
        
        // holds triangle vertices for now, but will contain mesh data later - shared for all
        GLuint VerticesBuffer = 0;
        GLuint OffsetBuffer = 0;
        GLuint IDBuffer = 0;

    };
}
