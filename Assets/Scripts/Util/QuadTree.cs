using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Profiling;

public unsafe partial class QuadTree
{
    // global storage of contained elements, should be more efficient than stored in each cell
    private NativeArray<Info> IDMapping;

    // TODO currently requires inline children, maybe move to LLS ptrs?
    private NativeArray<QuadTreeCell> Cells;
    private int EmptyIndex;
    // precomputed ranges, since the cells split regularly its depending on depth
    [ReadOnly] 
    private float[] Ranges;








    public void Init(int MaxID)
    {
        Profiler.BeginSample("ECS.QuadTree.Init");
        IDMapping = new NativeArray<Info>(MaxID, Allocator.Persistent);
        Cells = new NativeArray<QuadTreeCell>(1, Allocator.Persistent);
        Cells[0] = new()
        {
            Center = Vector3.zero, 
            FirstElement = EntityID.Invalid(),
            Depth = 0,
            PlaceableCount = 0,
            ParentID = INDEX_NONE,
            FirstChildID = INDEX_NONE
        };
        Ranges = new float[MaxDepth + 1];
        float CurRange = GlobalRange;
        for (int i = 0; i < Ranges.Length; i++)
        {
            Ranges[i] = CurRange;
            CurRange = CurRange / 2;
        }
        EmptyIndex = Cells.Length;
        Profiler.EndSample();
    }

    public void Add(ref EntityID ID, ref TransformComponent Transform)
    {
        if (ID.IsInvalid())
            return;

        // TODO kinda shitty performance, make add a single batch call
        Profiler.BeginSample("ECS.QuadTree.Add");
        Vector3 Pos = Transform.GetPosition();
        Profiler.BeginSample("ECS.QuadTree.Add.IDMapping");
        bool bContainsKey = IDMapping[ID.ID].bIsValid;
        Profiler.EndSample();
        Profiler.BeginSample("ECS.QuadTree.Add.Logic");
        int Target = bContainsKey ? IDMapping[ID.ID].CellID : INDEX_NONE;
        Profiler.EndSample();
        Profiler.BeginSample("ECS.QuadTree.Add.Contains");
        bool bIsUpdate = bContainsKey && Contains(Pos, Target) && Contains(ref ID, Target);
        Profiler.EndSample();

        if (bIsUpdate)
        {
            Profiler.BeginSample("ECS.QuadTree.Add.Update");
            var Temp = IDMapping[ID.ID];
            Temp.Transform = Transform;
            IDMapping[ID.ID] = Temp;
            Profiler.EndSample();
        }
        else
        {
            Profiler.BeginSample("ECS.QuadTree.ActualAdd");
            if (bContainsKey)
            {
                Delete(ref ID, IDMapping[ID.ID].CellID);
            }
            Target = Find(RootID, Pos);
            Add(ref ID, ref Transform, Target);
            Profiler.EndSample();
        }

        Profiler.EndSample();
    }

    public void Delete(ref EntityID ID)
    {
        if (!IDMapping[ID.ID].bIsValid)
            return;

        Delete(ref ID, IDMapping[ID.ID].CellID);
    }

    private void Add(ref EntityID ID, ref TransformComponent Transform, int BaseCell)
    {
        Profiler.BeginSample("ECS.QuadTree.AddInternal");
        int Target = Find(BaseCell, Transform.GetPosition());
        AddToCell(ref ID, ref Transform, Target);
        Profiler.EndSample();
    }

    private int CountIn(int CellID)
    {
        int Count = 0;

        EntityID CurrentID = Cells[CellID].FirstElement;
        while (!CurrentID.IsInvalid())
        {
            CurrentID = IDMapping[CurrentID.ID].NextEntity;
            Count++;
        }
        return Count;
    }

    private void AddToCell(ref EntityID ID, ref TransformComponent Transform, int CellID)
    {
        Profiler.BeginSample("ECS.QuadTree.Cell.Add");
        IDMapping[ID.ID] = new()
        {
            NextEntity = Cells[CellID].FirstElement,
            CellID = CellID,
            Transform = Transform,
            bIsValid = true
        };
        var TempCell = Cells[CellID];
        TempCell.FirstElement = ID;
        TempCell.PlaceableCount++;
        Cells[CellID] = TempCell;
        if (Cells[CellID].PlaceableCount > MaxPlaceableCount && Cells[CellID].Depth < MaxDepth)
        {
            Split(CellID);
        }
        Profiler.EndSample();
    }

    public List<int> FindRange(int CurrentID, Vector3 TargetPos, float Radius)
    {
        if (Cells[CurrentID].FirstChildID == INDEX_NONE)
            return new() { CurrentID };

        Profiler.BeginSample("ECS.QuadTree.Cell.FindRange");
        List<int> FoundCells = new();
        int Offset = Cells[CurrentID].FirstChildID;
        for (int i = 0; i < ChildCount; i++)
        {
            float Dis = Vector3.Distance(Cells[Offset + i].Center, TargetPos);
            if (Dis >= Radius)
                continue;

            FoundCells.AddRange(FindRange(Cells[Offset + i].ID, TargetPos, Radius));
        }
        Profiler.EndSample();
        return FoundCells;
    }

    public bool Verify()
    {
        for (int i = 0; i < IDMapping.Length; i++)
        {
            EntityID ID = new(i);
            if (!Contains(ref ID, IDMapping[ID.ID].CellID))
            {
                Debug.LogError("ID: " + ID + " is not contained in " + IDMapping[ID.ID].CellID);
                return false;
            }
            if (IDMapping[ID.ID].NextEntity.Equals(ID))
            {
                Debug.LogError("Selfreference in IDMap for " +ID);
                return false;
            }
        }
        for (int i = 0; i < EmptyIndex; i++)
        {
            EntityID Current = Cells[i].FirstElement;
            while (!Current.IsInvalid())
            {
                if (!Contains(ref Current, Cells[i].ID))
                {
                    Debug.LogError(Cells[i].ID + "Does not contain " + Current);
                    return false;
                }
                Current = IDMapping[Current.ID].NextEntity;
            }
        }
        return true;
    }

    private unsafe void Delete(ref EntityID ID, int CellID)
    {
        if (!IDMapping[ID.ID].bIsValid)
            return;

        Profiler.BeginSample("ECS.QuadTree.Cell.Delete");
        Profiler.BeginSample("ECS.QuadTree.Cell.Delete.Loop" + Cells[CellID].PlaceableCount);

        long Size = 4;
        EntityID* PrevPtr = (EntityID*)UnsafeUtility.Malloc(Size, 0, Allocator.Temp);
        PrevPtr->Invalidate();
        EntityID Current = Cells[CellID].FirstElement; 
        while (!Current.Equals(ID))
        {
            UnsafeUtility.MemCpy(PrevPtr, &Current, Size);
            Current = IDMapping[Current.ID].NextEntity;
        }
        Profiler.EndSample();

        if (Current.IsInvalid())
        {
            Debug.Log("Does not contain value anyway");
        }
        else
        {
            // the deleted element is the first, just update the FirstElem ptr
            if (PrevPtr->IsInvalid())
            {
                var Temp = Cells[CellID];
                Temp.FirstElement = IDMapping[Current.ID].NextEntity;
                Cells[CellID] = Temp;
            }
            else
            {
                var Temp = IDMapping[PrevPtr->ID];
                Temp.NextEntity = IDMapping[Current.ID].NextEntity;
                IDMapping[PrevPtr->ID] = Temp;
            }
            var TempC = Cells[CellID];
            TempC.PlaceableCount--;
            Cells[CellID] = TempC;
        }
        UnsafeUtility.Free(PrevPtr, Allocator.Temp);
        var TempID = IDMapping[ID.ID];
        TempID.bIsValid = false;
        IDMapping[ID.ID] = TempID;

        if (Cells[CellID].PlaceableCount == 0)
        {
            MergeCells(Cells[CellID].ParentID);
        }
        Profiler.EndSample();
    }


    private void MergeCells(int ParentID)
    {
        if (ParentID == INDEX_NONE)
            return;

        Profiler.BeginSample("ECS.QuadTree.MergeCells");
        // check if all neighbour children are deletable
        // aka empty and no further divisions
        int Offset = Cells[ParentID].FirstChildID;
        for (int i = 0; i < ChildCount; i++)
        {
            if (Cells[Offset + i].FirstChildID != INDEX_NONE || !Cells[Offset + i].FirstElement.IsInvalid())
            {
                Profiler.EndSample();
                return;
            }
        }

        // since all children have to be inlined in memory, we have to swap them 
        // all at once with the last cells, updating references.
        int FirstSelf = Cells[ParentID].FirstChildID;
        int FirstOther = EmptyIndex - ChildCount;
        int OtherParent = Cells[FirstOther].ParentID;
        int ParentParent = Cells[ParentID].ParentID;
        for (int i = 0; i < ChildCount; i++)
        {
            int CurrentSelf = FirstSelf + i;
            int CurrentOther = FirstOther + i;
            // update ptr in self
            var TempCopy = Cells[CurrentOther];
            TempCopy.ID = CurrentSelf;
            Cells[CurrentSelf] = TempCopy;

            // update ptr from contained elements
            EntityID CurrentID = Cells[CurrentOther].FirstElement;
            while (!CurrentID.IsInvalid())
            {
                var TempMap = IDMapping[CurrentID.ID];
                TempMap.CellID = CurrentSelf;
                IDMapping[CurrentID.ID] = TempMap;
                CurrentID = IDMapping[CurrentID.ID].NextEntity;
            }

            // if we swap parent and child position we need to remove the self ptr
            if (Cells[CurrentOther].FirstChildID == FirstSelf)
            {
                var TempCell = Cells[CurrentSelf];
                TempCell.FirstChildID = INDEX_NONE;
                Cells[CurrentSelf] = TempCell;
            }
            // update ptr from children
            int FirstOtherChild = Cells[CurrentSelf].FirstChildID;
            if (FirstOtherChild == INDEX_NONE)
                continue;

            for (int j = 0; j < ChildCount; j++)
            {
                var TempCell = Cells[FirstOtherChild + j];
                TempCell.ParentID = CurrentSelf;
                Cells[FirstOtherChild + j] = TempCell;
            }
        }
        // update ptr from parent
        var TempOther = Cells[OtherParent];
        TempOther.FirstChildID = FirstSelf;
        Cells[OtherParent] = TempOther;
        var TempParent = Cells[ParentID];
        TempParent.FirstChildID = INDEX_NONE;
        Cells[ParentID] = TempParent;
        EmptyIndex -= ChildCount;
        for (int i = EmptyIndex; i < EmptyIndex + ChildCount; i++)
        {
            Cells[i] = new();
        }
        MergeCells(ParentParent);
        Profiler.EndSample();
    }

    private void CreateCell(Vector3 Center, int ParentID, int Depth)
    {
        Profiler.BeginSample("ECS.QuadTree.CreateCell");
        Cells[EmptyIndex] = new()
        {
            Center = Center,
            ParentID = ParentID,
            ID = EmptyIndex,
            Depth = (byte)Depth,
            FirstElement = EntityID.Invalid(),
            FirstChildID = INDEX_NONE,
            PlaceableCount = 0
        };
        EmptyIndex++;
        Assert.IsTrue(EmptyIndex <= Cells.Length);
        Profiler.EndSample();
    }

    private void CreateCells(int ParentID)
    {
        Profiler.BeginSample("ECS.QuadTree.CreateCells");
        if (EmptyIndex >= Cells.Length - ChildCount)
        {
            Move();
        }
        /* Children are ordered as
            * bottom:      top:
            * 0 | 1        4 | 5
            * 2 | 3        6 | 7
        */
        int FirstIndex = EmptyIndex;
        Vector3 HalfRange = Ranges[Cells[ParentID].Depth] / 2 * Vector3.one;
        Vector3 Center = Cells[ParentID].Center;

        CreateCell(Center + new Vector3(-HalfRange.x, -HalfRange.y, +HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(+HalfRange.x, -HalfRange.y, +HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(-HalfRange.x, -HalfRange.y, -HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(+HalfRange.x, -HalfRange.y, -HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(-HalfRange.x, +HalfRange.y, +HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(+HalfRange.x, +HalfRange.y, +HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(-HalfRange.x, +HalfRange.y, -HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        CreateCell(Center + new Vector3(+HalfRange.x, +HalfRange.y, -HalfRange.z),
            ParentID, Cells[ParentID].Depth + 1);
        var Temp = Cells[ParentID];
        Temp.FirstChildID = FirstIndex;
        Cells[ParentID] = Temp;
        Profiler.EndSample();
    }


    private bool Contains(Vector3 Pos, int CellID)
    {

        Profiler.BeginSample("ECS.QuadTree.Contains");
        Vector3 Range = Ranges[Cells[CellID].Depth] * Vector3.one;
        bool bContains = 
            Cells[CellID].Center.x - Range.x < Pos.x && Pos.x < Cells[CellID].Center.x + Range.x &&
            Cells[CellID].Center.y - Range.y < Pos.y && Pos.y < Cells[CellID].Center.y + Range.y &&
            Cells[CellID].Center.z - Range.z < Pos.z && Pos.z < Cells[CellID].Center.z + Range.z;
        Profiler.EndSample();
        return bContains;
    }

    private bool Contains(ref EntityID ID, int CellID)
    {
        EntityID Current = Cells[CellID].FirstElement;
        while (!Current.IsInvalid())
        {
            if (Current.Equals(ID))
                return true;

            Current = IDMapping[Current.ID].NextEntity;
        }
        return false;
    }

    private void Split(int CellID)
    {
        Profiler.BeginSample("ECS.QuadTree.Cell.Split");

        CreateCells(CellID);
        while (!Cells[CellID].FirstElement.IsInvalid())
        {
            EntityID CurrentID = Cells[CellID].FirstElement;
            Info CurrentInfo = IDMapping[CurrentID.ID];
            Delete(ref CurrentID, CellID);
            Add(ref CurrentID, ref CurrentInfo.Transform, CellID);
            var Temp = Cells[CellID];
            Temp.FirstElement = CurrentInfo.NextEntity;
            Cells[CellID] = Temp;
        }
        Assert.IsTrue(Cells[CellID].FirstElement.IsInvalid());
        Profiler.EndSample();
    }

    /** Recursively finds the best-fit cell for the target pos, starting at the current cell index */
    public int Find(int CurrentIndex, Vector3 TargetPos)
    {
        if (Cells[CurrentIndex].FirstChildID == INDEX_NONE)
            return CurrentIndex;

        Profiler.BeginSample("ECS.QuadTree.Cell.Find");

        // since the layouts are fixed in memory, we can just jump there
        // see @QuadTree.CreateCells for info
        Vector3 Center = Cells[CurrentIndex].Center;
        bool X = TargetPos.x >= Center.x;
        bool Y = TargetPos.y >= Center.y;
        bool Z = TargetPos.z >= Center.z;

        int Target = (Y ? 4 : 0) +
                     (Z ? 0 : 2) +
                     (X ? 1 : 0);

        int Offset = Cells[CurrentIndex].FirstChildID;
        Profiler.EndSample();
        return Find(Cells[Offset + Target].ID, TargetPos);
    }

    private void Move()
    {
        Profiler.BeginSample("ECS.QuadTree.Move");
        int NewLength = Mathf.CeilToInt(Cells.Length * 1.5f);
        NewLength = Mathf.Min(NewLength, MaxCellCount);
        NewLength = Mathf.Max(NewLength, Cells.Length + MinCellCount);
        NativeArray<QuadTreeCell> NewCells = new(NewLength, Allocator.Persistent);
        var NewSlice = new NativeSlice<QuadTreeCell>(NewCells, 0, Cells.Length);
        var OldSlice = new NativeSlice<QuadTreeCell>(Cells);

        NewSlice.CopyFrom(OldSlice);
        Cells.Dispose();
        Cells = NewCells;
        Profiler.EndSample();
    }

    private unsafe struct Info
    {
        public bool bIsValid;
        public int CellID;
        // stores the next Entity thats also in the same cell (or invalid if the last element)
        public EntityID NextEntity;
        public TransformComponent Transform;
    }

    public void Dispose()
    {
        if (IDMapping != null)
        {
            IDMapping.Dispose();
        }
        if (Cells != null)
        {
            Cells.Dispose();
        }
    }

    public static float GlobalRange = 100;
    public static int MaxDepth = 6;
    protected static int MaxPlaceableCount = 10;
    protected static int ChildCount = 8;
    protected static int RootID = 0;
    protected static int INDEX_NONE = -1;
    protected static int MaxCellCount = (int)Mathf.Pow(ChildCount, MaxDepth) + 1;
    protected static int MinCellCount = ChildCount + 1;
}

public interface IPlaceable
{
    public Vector3 GetPosition();
}
