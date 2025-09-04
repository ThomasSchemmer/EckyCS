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
public class EckyCS : GameService, IComponentGroupViewProvider<SparseSet>
{
    private Dictionary<Type, List<EckyCSSystem>> Systems = new();

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

    public void OnDrawGizmos()
    {
        Profiler.BeginSample("ECS.OnDrawGizmos");
        ForEachSystem(System => System.OnDrawGizmos());
        Profiler.EndSample();
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
            Type GenType;
            switch (GenTypes.Length)
            {
                case 0: GenType = typeof(SparseSet); break;
                case 1: GenType = typeof(SparseSet<>).MakeGenericType(GenTypes); break;
                case 2: GenType = typeof(SparseSet<,>).MakeGenericType(GenTypes); break;
                case 3: GenType = typeof(SparseSet<,,>).MakeGenericType(GenTypes); break;
                default: throw new NotImplementedException();
            }
            var NewSet = (SparseSet)Activator.CreateInstance(GenType, new object[]
            {
                GroupID, GetSparseSetSize(), 10
            });
            EntitySets.Add(GroupID, NewSet);
        }
        return EntitySets[GroupID];
    }

    public void RegisterEntity(EntityID ID, ComponentGroupIdentifier GroupID, byte[] Data = null)
    {
        Assert.IsFalse(EntityMasks.ContainsKey(ID));
        GetSet(GroupID).Add(ID, Data);
        EntityMasks.Add(ID, GroupID);
    }

    public ComponentGroupIdentifier GetOrCreateGroup(EntityID ID)
    {
        if (!EntityMasks.ContainsKey(ID))
        {
            EntityMasks.Add(ID, EmptyGroup);
            EntitySets[EmptyGroup].Add(ID, null);
        }
        return EntityMasks[ID];
    }

    public bool TryGetSystem<T>(out T System) where T : EckyCSSystem
    {
        System = default;
        if (!TryGetSystems<T>(out var SystemList))
            return false;

        System = (T)SystemList[0];
        return true;
    }

    public bool TryGetSystems<T>(out List<EckyCSSystem> SystemList) where T : EckyCSSystem
    {
        SystemList = new();
        List<Type> Targets = new();
        foreach (var Key in Systems.Keys)
        {
            if (!typeof(T).IsAssignableFrom(Key))
                continue;

            Targets.Add(Key);
        }

        foreach (var Key in Targets)
        {
            SystemList.AddRange(Systems[Key]);
        }
        return SystemList.Count != 0;
    }


    protected override void ResetInternal()
    {
    }

    protected override void StartServiceInternal()
    {
        EntityMasks = new();
        EmptyGroup = new();
        GetSet(EmptyGroup);
        AddSystem(new LocationSystem());
        ForEachSystem(_ => _.StartSystem());
        _OnInit?.Invoke(this);
    }

    protected void ForEachSystem(Action<EckyCSSystem> Action)
    {
        foreach (var Entry in Systems)
        {
            foreach (var System in Entry.Value)
            {
                Action?.Invoke(System);
            }
        }
    }

    public void AddSystem(EckyCSSystem System)
    {
        var Type = System.GetType();
        if (!Systems.ContainsKey(Type))
        {
            Systems.Add(System.GetType(), new());
        }
        Systems[Type].Add(System);

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
            foreach (var System in Pair.Value)
            {
                System.Destroy();
            }
        }
    }

    public Dictionary<ComponentGroupIdentifier, SparseSet> GetViewSet()
    {
        return EntitySets;
    }

    public IComponentGroupViewProvider<SparseSet> GetProvider()
    {
        return this;
    }

    public const int MaxEntities = 50000;
}
