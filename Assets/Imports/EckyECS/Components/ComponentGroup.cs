using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Profiling;
using Unity.VisualScripting;
using System.Runtime.InteropServices;
using System.ComponentModel;

/** 
 * Dense struct to help have a variable amount of components in a sparse set
 * Sadly c# does not seem to support SparseSet<t..>, so we instead create a generic stub 
 * for each amount of types (x...z). Lots of boilerplate
 * Main advantage of this is that each component type has a dense array directly adjacent in memory (enabled with structs)
 * We also support the non-generic (aka no components per entity), which contains stubs for functions
 */
public class ComponentGroup
{
    public EntityID[] IDs;

    protected int ComponentAmount = -1;
    protected ComponentGroupIdentifier GroupID;
    protected int ComponentSize;

    protected virtual void ResetComponents(int Index, EntityID ID) { }
    protected virtual void ChangeSizeComponents(int NewLength) { }
    protected virtual void SwapComponents(int IndexA, int IndexB) { }
    public unsafe virtual byte* Get(int Index) { return null; }
    public unsafe virtual void ForEach(int Count, ByteAction Action) { }
    public unsafe virtual byte*[] GetGroupPointers() { return null; }

    public ComponentGroup(ComponentGroupIdentifier ID, int ExpectedEntities)
    {
        ComponentSize = 0;
        ComponentAmount = ID.GetAmountOfFlags();
        GroupID = ID;
        IDs = new EntityID[ExpectedEntities];
    }

    public void Reset(int Index, bool bResetComponents = true)
    {
        Set(Index, EntityID.Invalid(Index + 1), bResetComponents);
    }

    public void Set(int Index, EntityID ID, bool bResetComponents = true)
    {
        Assert.IsTrue(Index >= 0 && Index < IDs.Length);
        
        IDs[Index] = ID;

        if (!bResetComponents)
            return;
        
        ResetComponents(Index, ID);
    }


    public void ChangeSize(int NewLength)
    {
        Profiler.BeginSample("ECS.ComponentGroup.ChangeSize");
        int OldLength = Length();
        ChangeSizeIDs(NewLength);
        ChangeSizeComponents(NewLength);
        for (int e = OldLength; e < NewLength; e++)
        {
            Reset(e);
        }
        Profiler.EndSample();
    }

    private void ChangeSizeIDs(int NewLength)
    {
        EntityID[] NewIDs = new EntityID[NewLength];
        Array.Copy(IDs, NewIDs, IDs.Length);
        IDs = NewIDs;
    }

    public int GetGreaterLength()
    {
        int OldLength = IDs.Length;
        int AdditionalSize = (int)(OldLength * 0.5f);
        return AdditionalSize + OldLength;
    }

    public void Swap(int IndexA, int IndexB)
    {
        (IDs[IndexB], IDs[IndexA]) = (IDs[IndexA], IDs[IndexB]);
        SwapComponents(IndexA, IndexB);
    }

    public int Length()
    {
        return IDs.Length;
    }

    public unsafe delegate void ByteAction(EntityID ID, byte* ptr);
    public unsafe delegate void GroupByteAction(ComponentGroupIdentifier Group, byte*[] Ptrs, int Count);
}


public unsafe class ComponentGroup<T> : ComponentGroup where T : struct, IComponent
{
    // raw byte data as we cannot store it in T due to boxing
    public byte[] Components;

    public ComponentGroup(ComponentGroupIdentifier ID, int ExpectedEntities) : 
        base (ID, ExpectedEntities)
    {
        ComponentSize = Marshal.SizeOf(typeof(T));
        Components = new byte[ExpectedEntities * ComponentSize];

        for (int e = 0; e < ExpectedEntities; e++)
        {
            Reset(e, false);
        }

        unsafe
        {
            //fixed (byte* bPtr = &Components[0])
            //{
            //    TransformComponent* Ptr = (TransformComponent*)bPtr;
            //    TransformComponent* Ptr2 = (TransformComponent*)bPtr;
            //    Ptr2->PosX += 5;
            //
            //}
            //TransformComponent Test = (TransformComponent)Components[0];
            //TransformComponent* First = &Test;
            //First->Position = 0;
        }
    }
    public unsafe override byte* Get(int Index)
    {
        fixed (byte* bPtr = &Components[Index * ComponentSize])
        {
            return bPtr;
        }
    }

    public unsafe override void ForEach(int Count, ByteAction Action)
    {
        base.ForEach(Count, Action);
        fixed (byte* bPtr = &Components[0])
        {
            for (int i = 0; i < Count; i++)
            {
                {
                    Action(IDs[i], (bPtr + i * ComponentSize));
                }
            }
        }
    }

    public unsafe override byte*[] GetGroupPointers() {
        fixed (byte* Ptr = &Components[0])
        {
            return new byte*[] { Ptr };
        }
    }

    protected override void ChangeSizeComponents(int NewLength)
    {
        byte[] NewComponents = new byte[NewLength * ComponentSize];
        Array.Copy(Components, NewComponents, Components.Length);
        Components = NewComponents;
    }

    protected override void ResetComponents(int Index, EntityID ID)
    {
        int Offset = Index * ComponentSize;
        for (int i = 0; i < ComponentSize; i++)
        {
            Components[i + Offset] = 0;
        }
    }

    protected override void SwapComponents(int IndexA, int IndexB)
    {
        for (int i = 0; i < ComponentSize; i++)
        {
            int A = (IndexA * ComponentSize) + i;
            int B = (IndexB * ComponentSize) + i;
            (Components[A], Components[B]) = (Components[B], Components[A]);
        }
    }
}
