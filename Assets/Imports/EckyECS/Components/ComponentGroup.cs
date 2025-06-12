using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Profiling;
using static UnityEngine.GraphicsBuffer;

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

    protected virtual void ResetComponents(int Index, EntityID ID) { }
    protected virtual void ChangeSizeComponents(int NewLength) { }
    protected virtual void SwapComponents(int IndexA, int IndexB) { }
    public unsafe virtual byte* Get<T>(int Index) where T : struct, IComponent { return null; }
    public unsafe virtual void ForEach(ByteAction Action) { }
    public unsafe virtual byte*[] GetGroupPointers(Type[] Types) { return null; }

    public ComponentGroup(ComponentGroupIdentifier ID, int ExpectedEntities)
    {
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

    public bool Has(int TargetIndex)
    {
        return !IDs[TargetIndex].IsInvalid();
    }

    public unsafe delegate void ByteAction(ComponentGroupIdentifier Group, EntityID ID, byte*[] ptr);
    public unsafe delegate void GroupByteAction(ComponentGroupIdentifier Group, byte*[] Ptrs, int Count);
}


public unsafe class ComponentGroup<T> : ComponentGroup where T : struct, IComponent
{
    // raw byte data as we cannot store it in T due to boxing
    public byte[] Components;

    private int ComponentSize;

    public ComponentGroup(ComponentGroupIdentifier ID, int ExpectedEntities) : 
        base (ID, ExpectedEntities)
    {
        ComponentSize = Marshal.SizeOf(typeof(T));
        Components = new byte[ExpectedEntities * ComponentSize];

        for (int e = 0; e < ExpectedEntities; e++)
        {
            Reset(e, false);
        }

    }
    public unsafe override byte* Get<X>(int Index)
    {
        if (typeof(X) != typeof(T))
            return null;

        fixed (byte* bPtr = &Components[Index * ComponentSize])
        {
            return bPtr;
        }
    }

    public unsafe override void ForEach(ByteAction Action)
    {
        base.ForEach(Action);
        int Target = GroupID.GetSelfIndexOf(typeof(T));
        if (Target != 0)
            return;

        fixed (byte* bPtr = &Components[0])
        {
            for (int i = 0; i < IDs.Length; i++)
            {
                if (IDs[i].IsInvalid())
                    continue;

                Action(GroupID, IDs[i], new byte*[1] { bPtr + i * ComponentSize });
            }
        }
    }

    public unsafe override byte*[] GetGroupPointers(Type[] Types) 
    {
        int Target = GroupID.GetSelfIndexOf(typeof(T));
        if (Target < 0 || Target >= Components.Length)
            return null;

        fixed (byte* Ptr = &Components[Target])
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


public unsafe class ComponentGroup<X, Y> : ComponentGroup where X : struct, IComponent where Y : struct, IComponent
{
    // raw byte data as we cannot store it in T due to boxing
    public byte[] ComponentsX;
    public byte[] ComponentsY;

    private int ComponentSizeX, ComponentSizeY;

    public ComponentGroup(ComponentGroupIdentifier ID, int ExpectedEntities) :
        base(ID, ExpectedEntities)
    {
        ComponentSizeX = Marshal.SizeOf(typeof(X));
        ComponentsX = new byte[ExpectedEntities * ComponentSizeX];
        ComponentSizeY = Marshal.SizeOf(typeof(Y));
        ComponentsY = new byte[ExpectedEntities * ComponentSizeY];

        for (int e = 0; e < ExpectedEntities; e++)
        {
            Reset(e, false);
        }

    }

    public unsafe override byte* Get<T>(int Index)
    {
        int Target = GroupID.GetSelfIndexOf(typeof(T));
        if (Target < 0 || Target >= 2)
            return null;

        byte[] TargetPtr = Target == 0 ? ComponentsX : ComponentsY;
        int TargetSize = Target == 0 ? ComponentSizeX : ComponentSizeY;
        fixed (byte* bPtr = &TargetPtr[Index * TargetSize])
        {
            return bPtr;
        }
    }

    public unsafe override void ForEach(ByteAction Action)
    {
        base.ForEach(Action);
        fixed (byte* xPtr = &ComponentsX[0])
        {
            fixed (byte* yPtr = &ComponentsY[0])
            {
                for (int i = 0; i < IDs.Length; i++)
                {
                    if (IDs[i].IsInvalid())
                        continue;

                    Action(GroupID, IDs[i], new byte*[2]{
                        xPtr + i * ComponentSizeX,
                        yPtr + i * ComponentSizeY,
                    });   
                }
            }
        }
    }

    public unsafe override byte*[] GetGroupPointers(Type[] Types)
    {
        fixed (byte* xPtr = &ComponentsX[0])
        {
            fixed (byte* yPtr = &ComponentsY[0])
            {
                return new byte*[2]
                {
                    xPtr,
                    yPtr
                };
            }
        }
    }

    protected override void ChangeSizeComponents(int NewLength)
    {
        byte[] NewComponentsX = new byte[NewLength * ComponentSizeX];
        byte[] NewComponentsY = new byte[NewLength * ComponentSizeY];
        Array.Copy(ComponentsX, NewComponentsX, ComponentsX.Length);
        Array.Copy(ComponentsY, NewComponentsY, ComponentsY.Length);
        ComponentsX = NewComponentsX;
        ComponentsY = NewComponentsY;
    }

    protected override void ResetComponents(int Index, EntityID ID)
    {
        int OffsetX = Index * ComponentSizeX;
        for (int i = 0; i < ComponentSizeX; i++)
        {
            ComponentsX[i + OffsetX] = 0;
        }

        int OffsetY = Index * ComponentSizeY;
        for (int i = 0; i < ComponentSizeY; i++)
        {
            ComponentsY[i + OffsetY] = 0;
        }
    }

    protected override void SwapComponents(int IndexA, int IndexB)
    {
        for (int i = 0; i < ComponentSizeX; i++)
        {
            int A = (IndexA * ComponentSizeX) + i;
            int B = (IndexB * ComponentSizeX) + i;
            (ComponentsX[A], ComponentsX[B]) = (ComponentsX[B], ComponentsX[A]);
        }

        for (int i = 0; i < ComponentSizeY; i++)
        {
            int A = (IndexA * ComponentSizeY) + i;
            int B = (IndexB * ComponentSizeY) + i;
            (ComponentsY[A], ComponentsY[B]) = (ComponentsY[B], ComponentsY[A]);
        }
    }
}
