using System;
using System.Collections;
using System.Collections.Generic;
using static UnityEngine.GraphicsBuffer;

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
                    // todo: GetSelfIndex can be refactored out
                    int Target = GroupID.GetSelfIndexOf(typeof(TransformComponent));
                    TransformComponent* Component = (TransformComponent*)Ptr[Target];
                    Component->PosX = UnityEngine.Random.Range(0, 5f);
                    Component->PosZ = UnityEngine.Random.Range(0, 5f);
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
        // we actively need to query for completion
        foreach (var Pair in Trees)
        {
            Pair.Value.Tick(Delta);
        }
    }

    private unsafe void Register(ComponentGroupIdentifier Group, void*[] Ptrs, int Count)
    {
        if (!Trees.ContainsKey(Group))
        {
            Trees.Add(Group, new());
        }
        int CompTarget = Group.GetSelfIndexOf(typeof(TransformComponent));
        int IDsTarget = Ptrs.Length - 1;
        Trees[Group].Register(
            (TransformComponent*)Ptrs[CompTarget], 
            (EntityID*)Ptrs[IDsTarget], 
            Count
        );
        Trees[Group].Run();
    }

    public void OnDrawGizmos()
    {
        foreach (var Tuple in Trees)
        {
            Tuple.Value.Debug();
        }
    }


    public void Destroy() {

        foreach (var Tuple in Trees)
        {
            Trees[Tuple.Key].Destroy();
        }
        Trees.Clear();
    }
}
