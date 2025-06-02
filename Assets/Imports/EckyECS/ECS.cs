using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.Profiling;
using System.Runtime.InteropServices;
using UnityEngine.Assertions;

/**
 * Central class for the ECS - should be the only class that you directly interact with
 * Is a gameservice for added convenience 
 */
public class ECS : GameService
{
    private Dictionary<Type, ECSSystem> Systems = new();

    public Dictionary<ComponentGroupIdentifier, SparseSet> EntitySets = new();

    private Dictionary<EntityID, ComponentGroupIdentifier> EntityMasks;
    private ComponentGroupIdentifier EmptyGroup;

    public void Update()
    {
        Profiler.BeginSample("ECS.Update");
        ForEachSystem((System) => System.Tick(Time.deltaTime));
        Profiler.EndSample();
    }

    public void FixedUpdate()
    {
        Profiler.BeginSample("ECS.FixedUpdate");
        ForEachSystem(System => System.FixedTick(Time.fixedDeltaTime));
        Profiler.EndSample();
    }

    public void LateUpdate()
    {
        Profiler.BeginSample("ECS.LateUpdate");
        ForEachSystem(System => System.LateTick(Time.deltaTime));
        Profiler.EndSample();
    }
    public ComponentGroupView<X> Get<X>() where X : IComponent
    {
        Profiler.BeginSample("ECS.Get_X");
        ComponentGroupView<X> Groups = new();
        foreach (var Key in EntitySets.Keys)
        {
            if (!Key.HasFlag(typeof(X)))
                continue;

            Groups.Add(Key);
        }
        Profiler.EndSample();
        return Groups;
    }

    public ComponentGroupView<X, Y> Get<X, Y>() where X : IComponent where Y : IComponent
    {
        Profiler.BeginSample("ECS.Get_X_Y");
        ComponentGroupView<X, Y> Groups = new();
        List<Type> Reqs = new()
        {
            typeof(X),
            typeof(Y)
        };
        foreach (var Key in EntitySets.Keys)
        {
            if (!Key.HasAllFlags(Reqs) )
                continue;

            Groups.Add(Key);
        }
        Profiler.EndSample();
        return Groups;
    }

    public void AssignComponent<T>(EntityID ID) where T : IComponent
    {
        /*
        Profiler.BeginSample("ECS.AssignComponent");
        ComponentGroupIdentifier CurrentGroupID = GetOrCreateGroup(ID);

        var CurrentSet = GetSet(CurrentGroupID);
        ComponentGroup Group = CurrentSet.Get(ID);
        ComponentGroupIdentifier NewGroupID = GetGroupWith(CurrentGroupID, typeof(T));
        CurrentSet.Remove(ID);

        ComponentGroup NewGroup = ComponentGroup.CreateFrom(NewGroupID, Group, CurrentGroupID);

        unsafe
        {
            object[] objects = new object[] { Group, NewGroup };
            IntPtr* arrayPtr = (IntPtr*)Marshal.UnsafeAddrOfPinnedArrayElement(objects, 0);
            var objPtr = arrayPtr[0];
        }

        GetSet(NewGroupID).Add(NewGroup, ID);
        Profiler.EndSample();
        */
    }

    public ComponentGroupIdentifier GetGroupIDFromList(List<Type> Types)
    {
        ComponentGroupIdentifier Group = EmptyGroup.Clone();
        foreach (var Type in Types)
        {
            Group.AddFlag(Type);
        }
        return Group;
    }

    public ComponentGroupIdentifier GetGroupWith(ComponentGroupIdentifier CurrentGroup, Type ComponentType)
    {
        Profiler.BeginSample("ECS.GetGroupWith");
        if (CurrentGroup.HasFlag(ComponentType))
            return CurrentGroup;

        ComponentGroupIdentifier NewGroup = CurrentGroup.Clone();
        NewGroup.AddFlag(ComponentType);
        Profiler.EndSample();
        return NewGroup;
    }

    public SparseSet GetSet(ComponentGroupIdentifier GroupID)
    {
        if (!EntitySets.ContainsKey(GroupID))
        {
            Type[] GenTypes = GroupID.GetContainedTypes().ToArray();
            var GenType = GenTypes.Length > 0 ? typeof(SparseSet<>).MakeGenericType(GenTypes) : typeof(SparseSet);
            var NewSet = (SparseSet)Activator.CreateInstance(GenType, new object[]
            {
                GroupID, GetSparseSetSize(), 10
            });
            EntitySets.Add(GroupID, NewSet);
        }
        return EntitySets[GroupID];
    }

    public void RegisterEntity(EntityID ID, ComponentGroupIdentifier GroupID)
    {
        Assert.IsFalse(EntityMasks.ContainsKey(ID));
        GetSet(GroupID).Add(ID);
        EntityMasks.Add(ID, GroupID);
    }

    public ComponentGroupIdentifier GetOrCreateGroup(EntityID ID)
    {
        if (!EntityMasks.ContainsKey(ID))
        {
            EntityMasks.Add(ID, EmptyGroup);
            EntitySets[EmptyGroup].Add(ID);
        }
        return EntityMasks[ID];
    }

    public bool TryGetSystem<T>(Type Type, out T System) where T : ECSSystem
    {
        System = default;
        if (!Systems.ContainsKey(Type))
            return false;

        System = (T)Systems[Type];
        return true;
    }


    protected override void ResetInternal()
    {
    }

    protected override void StartServiceInternal()
    {
        EntityMasks = new();
        EmptyGroup = new();
        GetSet(EmptyGroup);
        ForEachSystem(_ => _.StartSystem());
        _OnInit?.Invoke(this);
    }

    protected void ForEachSystem(Action<ECSSystem> Action)
    {
        foreach (var Entry in Systems)
        {
            Action?.Invoke(Entry.Value);
        }
    }

    public void AddSystem(ECSSystem System)
    {
        Systems.Add(System.GetType(), System);
        if (!IsInit)
            return;

        System.StartSystem();
    }

    protected override void StopServiceInternal()
    {
    }

    private int GetSparseSetSize()
    {
        return Mathf.CeilToInt(MaxEntities / (float)SparseSet.PAGESIZE);
    }

    public void OnDestroy()
    {
        foreach (var Pair in Systems)
        {
            Pair.Value.Destroy();
        }
    }

    public const int MaxEntities = 50000;
}
