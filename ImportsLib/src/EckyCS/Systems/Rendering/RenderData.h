#pragma once

#include <glew/include/GL/glew.h>
#include "../../Components/ComponentGroupIdentifier.h"
#include "../../Components/Base/TransformComponent.h"
#include "../../Util/TypeInfo.h"

namespace EckyCS
{
    class GeometryProvider;
}

namespace EckyCS
{
    /**
    * Abstract helper class to manage data flow from the CPU to GPU
     * RenderData that has to explicitly copied to the GPU every frame
     * Used to integrate Entities into the Rendering Pipeline
     * Copies all Components specified by the targeted Entity type, but
     * has to use Ptr/lookup for non-standard ones
     */
    class RenderData
    {
    public:
        size_t Count;

        RenderData() = default;
        virtual ~RenderData() = default;
        virtual void Create(size_t count, const shared_ptr<GeometryProvider>& Provider);
        
        void Render() const;

        
        template <typename... Components>
        requires AllContainedIn<tuple<TransformComponent>, tuple<Components...>>
        void UpdateBuffers(ComponentGroupIdentifier GroupID, size_t InCount, View<Components...> Data)
        {
            auto IDPtr = GetID(Data).data();
            auto TransformPtr = Get<TransformComponent>(Data).data();
            
            glBindBuffer(GL_ARRAY_BUFFER, OffsetBuffer);
            void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
            memcpy(ptr, TransformPtr, InCount * sizeof(float) * 3);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, IDBuffer);
            ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
            memcpy(ptr, IDPtr, InCount * sizeof(int));
            glUnmapBuffer(GL_ARRAY_BUFFER);

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
                ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                memcpy(ptr, TargetPtr, InCount * Size);
                glUnmapBuffer(GL_ARRAY_BUFFER);
            }
        }
        
    protected:
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
