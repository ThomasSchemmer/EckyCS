#pragma once
#include <filesystem>
#include <fstream>

#include "../PlayerService.h"
#include "VoxReader.h"
#include "EckyCS/ECS.h"
#include "EckyCS/Entities/EntityGenerator.h"
#include "EckyCS/Entities/Plant.h"
#include "EckyCS/Entities/PlantTest.h"
#include "GameService/GameServiceDelegate.h"
#include "Util/VoxReader.h"

class PlayerService;

namespace GAS
{
    class GameplayAbilityComponent;
    class GameplayAbilitySystem;
}

using namespace GameImports;
using namespace std;
class RenderTestService : public GameService, public enable_shared_from_this<RenderTestService>
{
public:
    EntityID ID;
    
    RenderTestService() 
    {
        Type = GameServiceType::Test;
    }
    
    void StartServiceInternal () override
    {
        //LoadVoxelBlocks(25);
        
        ComponentGroupIdentifier GroupID;
        ItemComponent Comp;
        TransformComponent Transform;
        Transform.PosX = 0;
        Transform.PosY = 5;
        Transform.PosZ = 0;
        EntityGenerator::TryCreate<Plant>(OUT GroupID, OUT ID, Comp, Transform);
    }

    void LoadVoxel(int GroupTarget)
    {
        auto Voxels = VoxReader::LoadVoxFile();
        OnInit.ForEach(Type);
        TemplatedDelegate<ECS>::RunAfterServiceInit([&GroupTarget, &Voxels](const shared_ptr<ECS>& Ecs)
            {
                int GroupCount = 0;
                ItemComponent Comp;
                Comp.Type = 1;
                for (auto& Group : Voxels)
                {
                    for (auto& Transform : Group)
                    {
                        ComponentGroupIdentifier GroupID;
                        EntityID ID;
                        EntityGenerator::TryCreate<Plant>(OUT GroupID, OUT ID, Comp, Transform);
                    }
                    GroupCount++;
                    if (GroupCount == GroupTarget)
                        return;
                }
            },
            GameServiceType::Test,
            GameServiceType::EntityComponentSystem
        );
    }

    void LoadVoxelBlocks(int GroupTarget)
    {
        auto Voxels = VoxReader::LoadVoxFile();
        OnInit.ForEach(Type);
        TemplatedDelegate<ECS>::RunAfterServiceInit([&GroupTarget, &Voxels](const shared_ptr<ECS>& Ecs)
            {
                int GroupCount = 0;
                for (auto& Group : Voxels)
                {
                    bool bFirstHalf = GroupCount < GroupTarget / 2;
                    ItemComponent Temp{};
                    Temp.Type = bFirstHalf ? 0 : 1;
                    
                    ItemComponent* Comp = new ItemComponent[Group.size()];
                    fill_n(Comp, Group.size(), Temp);
                    if (bFirstHalf)
                    {
                        EntityGenerator::TryCreateMany<Plant>(Group.size(), Comp, Group.data());
                    }
                    else
                    {
                        Component* Comp2 = new Component[Group.size()];
                        EntityGenerator::TryCreateMany<PlantTest>(Group.size(), Comp, Group.data(), Comp2);
                        delete[] Comp2;
                    }
                    GroupCount++;
                    delete[] Comp;
                    if (GroupCount == GroupTarget)
                        return;
                }
            },
            GameServiceType::Test,
            GameServiceType::EntityComponentSystem
        );
    }


    
    void StopServiceInternal() override {}
    void ResetServiceInternal() override {}
};
