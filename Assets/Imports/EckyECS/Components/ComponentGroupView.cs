using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.Profiling;

public class ComponentGroupView
{
    public List<ComponentGroupIdentifier> Groups = new();

    public void Add(ComponentGroupIdentifier Group)
    {
        Groups.Add(Group);
    }
}

public class ComponentGroupView<X> : ComponentGroupView where X : IComponent
{
    public unsafe void ForEach(ComponentGroup.ByteAction Action)
    {
        if (!Game.TryGetService(out ECS ECS))
            return;

        foreach (var Group in Groups)
        {
            var SparseSet = ECS.EntitySets[Group];
            SparseSet.ForEach(Action);
        }
    }

    public unsafe void ForEachGroup(ComponentGroup.GroupByteAction Action)
    {
        if (!Game.TryGetService(out ECS ECS))
            return;

        foreach (var Group in Groups)
        {
            var SparseSet = ECS.EntitySets[Group];
            Action?.Invoke(Group, SparseSet.GetGroupPointers(), SparseSet.GetCount());
        }
    }
}

public class ComponentGroupView<X, Y> : ComponentGroupView where X: IComponent where Y : IComponent
{
    public void ForEach(Action<EntityID, X, Y> Action)
    {
        if (!Game.TryGetService(out ECS ECS))
            return;

        foreach (var Group in Groups)
        {
            var SparseSet = ECS.EntitySets[Group];
            //foreach (var Value in SparseSet)
            //{
            //    //Action?.Invoke(
            //    //    Value.ID, 
            //    //    (X)Value.Content[typeof(X)],
            //    //    (Y)Value.Content[typeof(Y)]
            //    //);
            //}
        }
    }
}
