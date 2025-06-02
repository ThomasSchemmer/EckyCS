using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Burst.Intrinsics;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Jobs;
using Unity.Jobs.LowLevel.Unsafe;
using UnityEngine;

public class FollowTargetSystem : ECSSystem
{
    private ECS ECS;

    public void StartSystem()
    {
        Game.RunAfterServiceInit((ECS ECS) =>
        {
            this.ECS = ECS;

            unsafe
            {
                ECS.Get<TransformComponent>().ForEach((ID, Ptr) =>
                {
                    TransformComponent* Component = (TransformComponent*)Ptr;
                    Component->PosX = ECS.MaxEntities - ID.ID;
                    Component->PosZ = ECS.MaxEntities - ID.ID;
                });
            }
        });

    }


    public void FixedTick(float Delta)
    {
        if (ECS == null)
            return;

        if (!ECS.TryGetSystem(typeof(BVHTree), out BVHTree Tree))
            return;

        unsafe
        {
            ECS.Get<TransformComponent>().ForEachGroup((Group, Ptrs, Count) =>
            {
                Tree.Register(Group, Ptrs, new List<int> { Count });
            });
        }
    }


    public void Destroy() { }
}
