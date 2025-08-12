using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;
using System;
using UnityEngine.Profiling;

/** 
 * Main structure to hold efficient data for entities
 * Has two containers: 
 *  - a sparse (paginated) mapping, where the @EntityID is the key
 *  - a dense class containing different @IComponent arrays with the actual values
 *  
 *  since we also support "empty" Components every function must be able to return / handle nulls!
 */
public class SparseSet
{
    public ComponentGroup Values;

    protected List<SparseSetPage> Pages;
    protected Dictionary<int, int> PageMapping = new();
    protected int NumPages;

    protected int Available = 0;
    protected EntityID NextAvailable;

    public int GetTargetIndex(EntityID ID)
    {
        var Page = GetPage(ID, true);
        var Index = GetIndexInPage(ID);
        return Page.Indices[Index];
    }

    public unsafe void ForEach(ComponentGroup.ByteAction Action)
    {
        Values?.ForEach(Action);
    }

    public unsafe void*[] GetGroupPointers(Type[] Types)
    {
        return Values?.GetGroupPointers(Types);
    }

    public int GetCount()
    {
        return Values.IDs.Length - Available;
    }

    public bool Has(EntityID ID)
    {
        int BaseIndex = GetBasePageIndex(ID);
        int MappedIndex = PageMapping.ContainsKey(BaseIndex) ? PageMapping[BaseIndex] : INVALID_INDEX;
        SparseSetPage Page = MappedIndex != INVALID_INDEX ? Pages[MappedIndex] : null;
        if (Page == null)
            return false;

        return Has(ID, Page, GetIndexInPage(ID));
    }

    protected int GetIndexInPage(EntityID ID)
    {
        return (int)(ID.ID % PAGESIZE);
    }

    protected int GetBasePageIndex(EntityID ID)
    {
        return (int)(ID / PAGESIZE);
    }

    protected SparseSetPage GetPage(EntityID ID, bool bShouldCreate)
    {
        var PageIndex = GetBasePageIndex(ID);
        if (!PageMapping.ContainsKey(PageIndex))
        {
            if (bShouldCreate)
            {
                CreatePageAt(PageIndex);
            }
            else
            {
                return null;
            }
        }
        return Pages[PageMapping[PageIndex]];
    }

    protected void CreatePageAt(int Index)
    {
        Assert.IsTrue(Index >= 0 && Index < Pages.Count);
        SparseSetPage Page = new()
        {

            Indices = new int[PAGESIZE],
        };
        System.Array.Fill(Page.Indices, INVALID_INDEX);
        int FreeIndex = GetFreePageIndex();
        Pages[FreeIndex] = Page;
        PageMapping.Add(Index, FreeIndex);
    }

    protected int GetFreePageIndex()
    {
        for (int i = 0; i < NumPages; i++)
        {
            if (Pages[i] == null)
                return i;
        }
        Assert.IsTrue(false, "Ran out of pages!");
        return -1;
    }

    public bool IsFull()
    {
        return Available == 0;
    }

    public SparseSet(ComponentGroupIdentifier ID, int NumPages, int ExpectedEntities)
    {
        this.NumPages = NumPages;
        Pages = new((int)NumPages);
        for (int i = 0; i < this.NumPages; i++)
        {
            Pages.Add(null);
        }
    }

    protected unsafe bool Has(EntityID ID, SparseSetPage Page, int IndexInPage)
    {
        int TargetIndex = Page.Indices[IndexInPage];
        bool bIsValid = TargetIndex != INVALID_INDEX && Values != null;
        if (!bIsValid || !Values.Has(TargetIndex))
            return false;

        return Values.IDs[TargetIndex].Equals(ID);
    }

    public void Add(EntityID ID, byte[] Data)
    {
        Profiler.BeginSample("ECS.SparseSet.Add");
        var Page = GetPage(ID, true);
        var Index = GetIndexInPage(ID);

        // already exists, just overwrite
        if (Has(ID, Page, Index))
        {
            Values.Set(Page.Indices[Index], ID);
            return;
        }

        if (IsFull())
        {
            Move();
        }

        Page.Indices[Index] = GetFreeValueIndex();
        Values?.Set(Page.Indices[Index], ID, true, Data);
        Profiler.EndSample();
    }

    public byte[] GetData(EntityID ID)
    {
        var Index = GetTargetIndex(ID);
        if (Index == -1)
            return null;

        return Values?.GetData(Index);
    }

    public void Remove(EntityID ID)
    {
        Profiler.BeginSample("ECS.SparseSet.Remove");
        Assert.IsTrue(Has(ID));
        int IndexInPage = GetIndexInPage(ID);
        var Page = GetPage(ID, false);
        int Index = Page.Indices[IndexInPage];

        EntityID Temp = NextAvailable;
        ID = new EntityID(ID.ID, ID.Version + 1);
        NextAvailable = ID;
        if (Values != null)
        {
            Values.Reset(Index);
            Values.IDs[Index] = Temp;
        }
        Available++;
        Profiler.EndSample();
    }

    public void Swap(EntityID A, EntityID B)
    {
        SparseSetPage PageA = GetPage(A, false);
        SparseSetPage PageB = GetPage(B, false);
        int IndexInPageA = GetIndexInPage(A);
        int IndexInPageB = GetIndexInPage(B);
        int DenseIndexA = PageA.Indices[IndexInPageA];
        int DenseIndexB = PageB.Indices[IndexInPageB];
        Values?.Swap(DenseIndexA, DenseIndexB);
        PageA.Indices[IndexInPageA] = DenseIndexB;
        PageB.Indices[IndexInPageB] = DenseIndexA;
    }


    public void Shrink()
    {
        if (Values == null)
            return;

        // make implicit list of available slots explicit
        // since we want to cut them off at the end we don't need to recreate and can ignore the implicit ordering
        EntityID Current = NextAvailable;
        List<EntityID> IDsToSwap = new() { };
        for (int i = 0; i < Available; i++)
        {
            IDsToSwap.Add(Current);
            int TargetIndex = GetTargetIndex(Current);
            Current = Values.IDs[TargetIndex];
        }

        // reset implicit values to make swapping / reverse lookup easier
        foreach (var ID in IDsToSwap)
        {
            int TargetIndex = GetTargetIndex(ID);
            Values.IDs[TargetIndex] = ID;
        }

        // now we can easily swap all "invalid" elements to the back
        for (int i = 0; i < Available; i++)
        {
            EntityID B = Values.IDs[^(i + 1)];
            bool bIsContained = IDsToSwap.Contains(B);
            if (bIsContained)
            {
                IDsToSwap.Remove(B);
                SparseSetPage PageB = GetPage(B, false);
                int IndexInPageB = GetIndexInPage(B);
                PageB.Indices[IndexInPageB] = INVALID_INDEX;
            }
            else
            {
                EntityID A = IDsToSwap[0];
                Swap(A, B);
                IDsToSwap.Remove(A);
                SparseSetPage PageA = GetPage(A, false);
                int IndexInPageA = GetIndexInPage(A);
                PageA.Indices[IndexInPageA] = INVALID_INDEX;
            }
        }

        int OldLength = Values.Length();
        int NewLength = OldLength - Available;
        Values.ChangeSize(NewLength);
        Available = 0;
        NextAvailable = EntityID.Invalid(Values.Length());
    }

    protected int GetFreeValueIndex()
    {
        if (Values == null)
            return INVALID_INDEX;

        Assert.IsFalse(Has(NextAvailable));
        int TargetIndex = GetTargetIndex(NextAvailable);
        NextAvailable = Values.IDs[TargetIndex];
        Available--;
        return TargetIndex;
    }

    protected void Move()
    {
        if (Values == null)
            return;

        Profiler.BeginSample("ECS.SparseSet.Move");
        int OldLength = Values.Length();
        int NewLength = Values.GetGreaterLength();
        NewLength = (int)Mathf.Min(NewLength, PAGESIZE * Pages.Count);
        Values.ChangeSize(NewLength);
        FillWithEmpty(OldLength, NewLength);
        Profiler.EndSample();
    }

    protected void FillWithEmpty(int Start, int End)
    {
        for (int i = Start; i < End; i++)
        {
            // we have to setup redirectors to the next element,
            // creating the "ToDelete" indirect list
            EntityID Temp = EntityID.Invalid(i);
            var Page = GetPage(Temp, true);
            var Index = GetIndexInPage(Temp);
            Page.Indices[Index] = i;
        }
        NextAvailable = EntityID.Invalid(Start);
        Available += (End - Start);
    }

    protected void DeletePageAt(EntityID ID)
    {
        int Index = GetBasePageIndex(ID);
        int ActualIndex = PageMapping[Index];
        // Assert.IsTrue(Pages[ActualIndex].DenseIDs.Count == 0, "Cannot delete non-empty pages!");
        Pages[ActualIndex] = null;
        PageMapping.Remove(Index);
    }


    public static uint PAGESIZE = 256;
    public static int INVALID_INDEX = -1;
}

public class SparseSet<T> : SparseSet where T : struct, IComponent
{
    public SparseSet(ComponentGroupIdentifier ID, int NumPages, int ExpectedEntities) :
        base(ID, NumPages, ExpectedEntities)
    {
        Values = new ComponentGroup<T>(ID, ExpectedEntities);
        FillWithEmpty(0, ExpectedEntities);
    }
}
public class SparseSet<X, Y> : SparseSet where X : struct, IComponent where Y : struct, IComponent
{
    public SparseSet(ComponentGroupIdentifier ID, int NumPages, int ExpectedEntities) :
        base(ID, NumPages, ExpectedEntities)
    {
        Values = new ComponentGroup<X, Y>(ID, ExpectedEntities);
        FillWithEmpty(0, ExpectedEntities);
    }
}
public class SparseSet<X, Y, Z> : SparseSet where X : struct, IComponent where Y : struct, IComponent where Z : struct, IComponent
{
    public SparseSet(ComponentGroupIdentifier ID, int NumPages, int ExpectedEntities) :
        base(ID, NumPages, ExpectedEntities)
    {
        Values = new ComponentGroup<X, Y, Z>(ID, ExpectedEntities);
        FillWithEmpty(0, ExpectedEntities);
    }
}


public class SparseSetPage
{
    public int[] Indices;
}