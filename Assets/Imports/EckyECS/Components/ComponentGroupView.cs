using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.Profiling;

/** 
 * Virtual collection of CGIs that fulfill a common required set of Components
 * Used to quickly iterate/act on these components ("for each")
 */
public abstract class ComponentGroupView
{
    public List<ComponentGroupIdentifier> Groups = new();

    public void Add(ComponentGroupIdentifier Group)
    {
        Groups.Add(Group);
    }

    /** 
     * Executes the provided action for each entity from each group
     * Can be imperformant as it can create a lot of calls
     */
    public virtual unsafe void ForEach(ComponentGroup.ByteAction Action)
    {
        if (!Game.TryGetService(out ECS ECS))
            return;

        foreach (var Group in Groups)
        {
            var SparseSet = ECS.EntitySets[Group];
            SparseSet.ForEach(Action);
        }
    }

    /**
     * Executes the provided action once for each contained group
     * Can be performant as it allows to directly work on memory blocks instead
     */
    public unsafe void ForEachGroup(ComponentGroup.GroupByteAction Action)
    {
        if (!Game.TryGetService(out ECS ECS))
            return;

        foreach (var Group in Groups)
        {
            var SparseSet = ECS.EntitySets[Group];
            Action?.Invoke(
                Group, 
                SparseSet.GetGroupPointers(GetTypeSet()),
                SparseSet.GetCount()
            );
        }
    }

    public unsafe bool CheckEachGroup(ComponentGroup.GroupByteCheck Action, bool bExpectedResult)
    {
        if (!Game.TryGetService(out ECS ECS))
            return false;

        foreach (var Group in Groups)
        {
            var SparseSet = ECS.EntitySets[Group];
            bool bResult = (bool)Action?.Invoke(
                Group,
                SparseSet.GetGroupPointers(GetTypeSet()),
                SparseSet.GetCount()
            );
            if (bResult != bExpectedResult)
                return false;
        }

        return true;
    }

    public abstract Type[] GetTypeSet();
}

public class ComponentGroupView<X> : ComponentGroupView where X : IComponent
{
    public override Type[] GetTypeSet()
    {
        return new Type[1] { typeof(X) };
    }

}

public class ComponentGroupView<X, Y> : ComponentGroupView where X: IComponent where Y : IComponent
{

    public override Type[] GetTypeSet()
    {
        return new Type[2] { typeof(X), typeof(Y) };
    }
}
