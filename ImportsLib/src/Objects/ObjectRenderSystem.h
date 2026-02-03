#pragma once
#include "Object.h"
#include "ObjectRenderData.h"
#include "../EckyCS/Systems/Rendering/EntityRenderSystem.h"
#include "../EckyCS/Systems/Rendering/GLTFGeometryProvider.h"
#include "../Renderer/Renderer.h"

/**
 * Idea: iterate over available models.rc and load all data for it
 * then provide utility for placing and instantiate all placed objects
 * via lookup and amulti instanced draw call?
 */
class ObjectRenderSystem : public EntityRenderSystem<Object, ObjectRenderData, GLTFGeometryProvider>
{
public:

    bool SupportsRenderPass(RenderPassType Type) const override
    {
        return Type == RenderPassType::BasePass;
    }

    void Tick(float Delta) override;
    void StartSystem(shared_ptr<ECS>& Ecs) override;

private:
    std::shared_ptr<ECS> EcsPtr;
};
