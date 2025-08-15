using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Unity.Collections.LowLevel.Unsafe;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Profiling;

/** 
 * Dense struct to help have a variable amount of components in a sparse set
 * Sadly c# does not seem to support SparseSet<t..>, so we instead create a generic stub 
 * for each amount of types (x...z). Lots of boilerplate
 * Main advantage of this is that each component type has a dense array directly adjacent in memory (enabled with structs)
 * We also support the non-generic (aka no components per entity), which contains stubs for functions
 */
public abstract class ComponentGroup
{
    public EntityID[] IDs;

    protected int ComponentAmount = -1;
    protected ComponentGroupIdentifier GroupID;

    /** Returns a ptr to the requested component at index */
    public unsafe abstract void* Get<T>(int Index) where T : struct, IComponent;
    /** Executes the provided action for each valid entity in the group */
    public unsafe abstract void ForEach(ByteAction Action);
    /** Returns an array of ptrs to any components[] requested via Types, as well as to the ID array*/
    public unsafe abstract void*[] GetGroupPointers();

    public abstract void SetData(int Index, byte[] Data);
    public abstract byte[] GetData(int Index);
    protected abstract void ResetComponents(int Index, EntityID ID);
    protected abstract void ChangeSizeComponents(int NewLength);
    protected abstract void SwapComponents(int IndexA, int IndexB);

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

    public void Set(int Index, EntityID ID, bool bResetComponents = true, byte[] Data = null)
    {
        Assert.IsTrue(Index >= 0 && Index < IDs.Length);
        
        IDs[Index] = ID;

        if (bResetComponents)
        {
            ResetComponents(Index, ID);
        }
        if (Data != null)
        {
            SetData(Index, Data);
        }
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

    public unsafe delegate void ByteAction(ComponentGroupIdentifier Group, EntityID ID, void*[] ptr);
    public unsafe delegate void GroupByteAction(ComponentGroupIdentifier Group, void*[] Ptrs, int Count);
    public unsafe delegate bool GroupByteCheck(ComponentGroupIdentifier Group, void*[] Ptrs, int Count);
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

    public override void SetData(int Index, byte[] Data)
    {
        if (Data.Length != ComponentSize)
            return;

        Array.Copy(Data, 0, Components, Index * ComponentSize, ComponentSize);
    }

    public override byte[] GetData(int Index)
    {
        byte[] Data = new byte[ComponentSize];
        Array.Copy(Components, Index * ComponentSize, Data, 0, ComponentSize);
        return Data;
    }

    public unsafe override void* Get<X>(int Index)
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
        fixed (byte* bPtr = &Components[0])
        {
            for (int i = 0; i < IDs.Length; i++)
            {
                if (IDs[i].IsInvalid())
                    continue;

                Action(GroupID, IDs[i], new void*[1] { bPtr + i * ComponentSize });
            }
        }
    }

    public unsafe override void*[] GetGroupPointers() 
    {
        // can't use a list for void*
        void*[] Result = new void*[1 + 1];
        int ResultIndex = 0;
        fixed (byte* xPtr = &Components[0])
        {
            Result[ResultIndex++] = xPtr;
        }
        fixed (EntityID* IDPtr = &IDs[0])
        {
            Result[ResultIndex++] = IDPtr;
        }
        return Result;
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

    private readonly int ComponentSizeX, ComponentSizeY;

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

    public unsafe override void* Get<T>(int Index)
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

    public override void SetData(int Index, byte[] Data)
    {
        int TotalSize = ComponentSizeX + ComponentSizeY;
        if (Data.Length != TotalSize)
            return;

        Array.Copy(Data, 0, ComponentsX, Index * TotalSize, ComponentSizeX);
        Array.Copy(Data, ComponentSizeX, ComponentsY, Index * TotalSize + ComponentSizeX, ComponentSizeY);
    }

    public override byte[] GetData(int Index)
    {
        byte[] Data = new byte[ComponentSizeX + ComponentSizeY];
        Array.Copy(ComponentsX, Index * ComponentSizeX, Data, 0, ComponentSizeX);
        Array.Copy(ComponentsY, Index * ComponentSizeY, Data, ComponentSizeX, ComponentSizeY);
        return Data;
    }

    public unsafe override void ForEach(ByteAction Action)
    {
        fixed (byte* xPtr = &ComponentsX[0])
        {
            fixed (byte* yPtr = &ComponentsY[0])
            {
                for (int i = 0; i < IDs.Length; i++)
                {
                    if (IDs[i].IsInvalid())
                        continue;

                    Action(GroupID, IDs[i], new void*[2]{
                        xPtr + i * ComponentSizeX,
                        yPtr + i * ComponentSizeY,
                    });   
                }
            }
        }
    }

    public unsafe override void*[] GetGroupPointers()
    {
        // can't use a list for void*
        void*[] Result = new void*[2 + 1];
        int ResultIndex = 0;
        fixed (byte* xPtr = &ComponentsX[0])
        {
            Result[ResultIndex++] = xPtr;
        }
        fixed (byte* yPtr = &ComponentsY[0])
        {
            Result[ResultIndex++] = yPtr;
        }
        fixed (EntityID* IDPtr = &IDs[0])
        {
            Result[ResultIndex++] = IDPtr;
        }
        return Result;
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

public unsafe class ComponentGroup<X, Y, Z> : ComponentGroup where X : struct, IComponent where Y : struct, IComponent where Z : struct, IComponent
{
    // raw byte data as we cannot store it in T due to boxing
    public byte[] ComponentsX;
    public byte[] ComponentsY;
    public byte[] ComponentsZ;

    private readonly int ComponentSizeX, ComponentSizeY, ComponentSizeZ;

    public ComponentGroup(ComponentGroupIdentifier ID, int ExpectedEntities) :
        base(ID, ExpectedEntities)
    {
        ComponentSizeX = Marshal.SizeOf(typeof(X));
        ComponentsX = new byte[ExpectedEntities * ComponentSizeX];
        ComponentSizeY = Marshal.SizeOf(typeof(Y));
        ComponentsY = new byte[ExpectedEntities * ComponentSizeY];
        ComponentSizeZ = Marshal.SizeOf(typeof(Z));
        ComponentsZ = new byte[ExpectedEntities * ComponentSizeZ];

        for (int e = 0; e < ExpectedEntities; e++)
        {
            Reset(e, false);
        }
    }

    public unsafe override void* Get<T>(int Index)
    {
        int Target = GroupID.GetSelfIndexOf(typeof(T));
        if (Target < 0 || Target >= 2)
            return null;

        byte[] TargetPtr;
        int TargetSize; 
        switch (Target)
        {
            case 0: TargetPtr = ComponentsX; TargetSize = ComponentSizeX; break;
            case 1: TargetPtr = ComponentsY; TargetSize = ComponentSizeY; break;
            case 2: TargetPtr = ComponentsZ; TargetSize = ComponentSizeZ; break;
            default: throw new System.Exception("Invalid");
        }

        fixed (byte* bPtr = &TargetPtr[Index * TargetSize])
        {
            return bPtr;
        }
    }

    public unsafe override void ForEach(ByteAction Action)
    {
        fixed (byte* xPtr = &ComponentsX[0])
        {
            fixed (byte* yPtr = &ComponentsY[0])
            {
                fixed (byte* zPtr = &ComponentsZ[0])
                {
                    for (int i = 0; i < IDs.Length; i++)
                    {
                        if (IDs[i].IsInvalid())
                            continue;

                        Action(GroupID, IDs[i], new void*[3]{
                        xPtr + i * ComponentSizeX,
                        yPtr + i * ComponentSizeY,
                        zPtr + i * ComponentSizeZ,
                    });
                    }
                }
            }
        }
    }

    public override void SetData(int Index, byte[] Data)
    {
        int TotalSize = ComponentSizeX + ComponentSizeY + ComponentSizeZ;
        if (Data.Length != TotalSize)
            return;

        Array.Copy(
            Data, 0, 
            ComponentsX, Index * ComponentSizeX, 
            ComponentSizeX
        );
        Array.Copy(
            Data, ComponentSizeX, 
            ComponentsY, Index * ComponentSizeY, 
            ComponentSizeY
        );
        Array.Copy(
            Data, ComponentSizeX + ComponentSizeY, 
            ComponentsZ, Index * ComponentSizeZ, 
            ComponentSizeZ
        );
    }

    public override byte[] GetData(int Index)
    {
        byte[] Data = new byte[ComponentSizeX + ComponentSizeY + ComponentSizeZ];
        Array.Copy(ComponentsX, Index * ComponentSizeX, Data, 0, ComponentSizeX);
        Array.Copy(ComponentsY, Index * ComponentSizeY, Data, ComponentSizeX, ComponentSizeY);
        Array.Copy(ComponentsZ, Index * ComponentSizeZ, Data, ComponentSizeX + ComponentSizeY, ComponentSizeZ);
        return Data;
    }

    public unsafe override void*[] GetGroupPointers()
    {
        void*[] Result = new void*[3 + 1];
        int ResultIndex = 0;
        fixed (byte* xPtr = &ComponentsX[0])
        {
            Result[ResultIndex++] = xPtr;
        }
        fixed (byte* yPtr = &ComponentsY[0])
        {
            Result[ResultIndex++] = yPtr;
        }
        fixed (byte* zPtr = &ComponentsZ[0])
        {
            Result[ResultIndex++] = zPtr;
        }
        fixed (EntityID* IDPtr = &IDs[0])
        {
            Result[ResultIndex++] = IDPtr;
        }
        return Result;
    }

    protected override void ChangeSizeComponents(int NewLength)
    {
        byte[] NewComponentsX = new byte[NewLength * ComponentSizeX];
        byte[] NewComponentsY = new byte[NewLength * ComponentSizeY];
        byte[] NewComponentsZ = new byte[NewLength * ComponentSizeZ];
        Array.Copy(ComponentsX, NewComponentsX, ComponentsX.Length);
        Array.Copy(ComponentsY, NewComponentsY, ComponentsY.Length);
        Array.Copy(ComponentsZ, NewComponentsZ, ComponentsZ.Length);
        ComponentsX = NewComponentsX;
        ComponentsY = NewComponentsY;
        ComponentsZ = NewComponentsZ;
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

        int OffsetZ = Index * ComponentSizeZ;
        for (int i = 0; i < ComponentSizeZ; i++)
        {
            ComponentsZ[i + OffsetZ] = 0;
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

        for (int i = 0; i < ComponentSizeZ; i++)
        {
            int A = (IndexA * ComponentSizeZ) + i;
            int B = (IndexB * ComponentSizeZ) + i;
            (ComponentsZ[A], ComponentsZ[B]) = (ComponentsZ[B], ComponentsZ[A]);
        }
    }

}