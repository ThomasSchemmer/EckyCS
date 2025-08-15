using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/** 
 * Creates a BVH tree for each group with a TransformComponent, 
 * allowing easier location querying
 */
public class LocationSystem : ECSSystem, IComponentGroupViewProvider<BVHTree>
{
    private ECS ECS;

    private Dictionary<ComponentGroupIdentifier, BVHTree> Trees;

    public void StartSystem()
    {
        Game.RunAfterServiceInit((ECS ECS) =>
        {
            this.ECS = ECS;
            Trees = new();
        });

    }


    public void FixedTick(float Delta)
    {
        if (ECS == null)
            return;

        unsafe
        {
            ECS.GetProvider().Get<TransformComponent>().ForEachGroup((Group, Ptrs, Count) =>
            {
                Register(Group, Ptrs, Count);
            });
        }
    }

    public void Tick(float Delta)
    {
        if (Trees == null)
            return;

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
        int TrasnformTarget = Group.GetSelfIndexOf(typeof(TransformComponent));
        int IDsTarget = Ptrs.Length - 1;
        Trees[Group].Register(
            (TransformComponent*)Ptrs[TrasnformTarget], 
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

    public bool IsEntityAt<X, Y>(Vector3 Location) where X : IComponent where Y : IComponent
    {
        foreach (var Group in GetProvider().Get<X, Y>().Groups)
        {
            if (!Trees.ContainsKey(Group))
                continue;

            // only interested in X/Z coords
            Vector3 Size = Vector3.one * 0.1f;
            Vector3 TempPos = Location - Size / 2; 
            Rect Temp = new(new Vector2(TempPos.x, TempPos.z), new(Size.x, Size.z));
            var List = Trees[Group].GetAllAt(Temp);
            if (List.Count > 0)
                return true;
        }
        return false;
    }

    public bool IsEntityAt<X>(Vector3 Location) where X : IComponent 
    {
        foreach (var Group in GetProvider().Get<X>().Groups)
        {
            if (!Trees.ContainsKey(Group))
                continue;

            // only interested in X/Z coords
            Vector3 Size = Vector3.one * 0.1f;
            Vector3 TempPos = Location - Size / 2;
            Rect Temp = new(new Vector2(TempPos.x, TempPos.z), new(Size.x, Size.z));
            var List = Trees[Group].GetAllAt(Temp);
            if (List.Count > 0)
                return true;
        }
        return false;
    }



    public Dictionary<ComponentGroupIdentifier, BVHTree> GetViewSet()
    {
        return Trees;
    }

    public IComponentGroupViewProvider<BVHTree> GetProvider()
    {
        return this;
    }
}
