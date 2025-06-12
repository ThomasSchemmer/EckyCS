using System;
using System.Collections;
using System.Collections.Generic;

/** 
 * Creates a BVH tree for each group with a TransformComponent, 
 * allowing easier location querying
 */
public class LocationSystem : ECSSystem
{
    private ECS ECS;

    private Dictionary<ComponentGroupIdentifier, BVHTree> Trees;

    public void StartSystem()
    {
        Game.RunAfterServiceInit((ECS ECS) =>
        {
            this.ECS = ECS;
            Trees = new();
            unsafe
            {
                ECS.Get<TransformComponent>().ForEach((GroupID, ID, Ptr) =>
                {
                    int Target = GroupID.GetSelfIndexOf(typeof(TransformComponent));
                    TransformComponent* Component = (TransformComponent*)Ptr[Target];
                    Component->PosX = ID.ID;
                    Component->PosZ = ID.ID;
                });
            }
        });

    }


    public void FixedTick(float Delta)
    {
        if (ECS == null)
            return;

        unsafe
        {
            ECS.Get<TransformComponent>().ForEachGroup((Group, Ptrs, Count) =>
            {
                Register(Group, Ptrs, Count);
            });
        }
    }

    public void Tick(float Delta)
    {
        foreach (var Pair in Trees)
        {
            Pair.Value.Tick(Delta);
        }
    }

    private unsafe void Register(ComponentGroupIdentifier Group, byte*[] Ptrs, int Count)
    {
        if (!Trees.ContainsKey(Group))
        {
            Trees.Add(Group, new());
        }
        int Target = Group.GetSelfIndexOf(typeof(TransformComponent));
        Trees[Group].Register((TransformComponent*)Ptrs[Target], Count);
        Trees[Group].Run();
    }


    public void Destroy() { }
}
