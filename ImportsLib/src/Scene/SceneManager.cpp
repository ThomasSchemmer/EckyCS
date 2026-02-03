#include "SceneManager.h"

#include <memory>

#include "../EckyCS/ECS.h"
#include "../GameService/Game.h"
#include "../Objects/ObjectRenderSystem.h"
#include "../Util/ShaderHelper.h"
#include "gltf/gltf.hpp"

namespace Scene
{
    void SceneManager::Init()
    {
        auto EcsPtr = Game::GetService<ECS>(GameServiceType::EntityComponentSystem);
        if (!EcsPtr)
            return;

        EcsPtr->AddSystem(std::make_shared<ObjectRenderSystem>());
    }
}
