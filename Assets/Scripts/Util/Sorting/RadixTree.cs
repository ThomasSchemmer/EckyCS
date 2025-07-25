using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using Unity.Burst;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Jobs;
using UnityEngine;

/** 
 * Helper struct to create a radix tree from an already sorted array of morton codess
 * implements https://developer.nvidia.com/blog/parallelforall/wp-content/uploads/2012/11/karras2012hpg_paper.pdf
 */
public struct RadixTree
{
    [BurstCompile]
    /** For every Node, calculate its bounds and split from the MortonCodes */
    public unsafe struct Create : IJobParallelFor
    {
        [ReadOnly]
        public NativeArray<uint> MortonCodes;

        [NativeDisableParallelForRestriction]
        public NativeArray<Node> Nodes;


        [NativeDisableParallelForRestriction]
        /** To avoid reinitializing we set index 0 => no parent */
        public NativeArray<int> MortonParents;

        public int ThreadCount;
        private int CountPerThread;
        [NativeDisableUnsafePtrRestriction]
        private Node* NodePtr;

        public void Execute(int i)
        {
            CountPerThread = Nodes.Length / ThreadCount;
            NodePtr = (Node*)Nodes.GetUnsafePtr();
            int LeftOverAmount = Nodes.Length - ThreadCount * CountPerThread;
            for (int j = 0; j < CountPerThread; j++)
            {
                Execute(i, j);
            }

            if (i >= LeftOverAmount)
                return;
            
            Execute(ThreadCount, i);
        }

        private void Debug(int Index, bool bIsLeaf)
        {
            if (bIsLeaf)
            {
                UnityEngine.Debug.Log(MortonCodes[Index]);
                return;
            }
            Nodes[Index].GetLeft(out var LeftIndex, out var LeftLeaf);
            Nodes[Index].GetRight(out var RightIndex, out var RightLeaf);
            Debug(LeftIndex, LeftLeaf);
            Debug(RightIndex, RightLeaf);
        }

        private void Execute(int ThreadIndex, int LoopIndex)
        {
            // see paper for explanation
            int Offset = ThreadIndex * CountPerThread + LoopIndex;
            int Dir = GetDirection(Offset);
            int Range = GetNodeRange(Offset, Dir);
            int End = Offset + Range * Dir;
            int PrefixNode = GetPrefixLength(Offset, End);
            int Split = GetNodeSplit(Offset, Range, Dir, PrefixNode);

            NodePtr[Offset].First = Offset;
            NodePtr[Offset].Last = End;
            NodePtr[Offset].Split = Split;

            SetParents(Offset);
        }

        private void SetParents(int Offset)
        {
            NodePtr[Offset].GetLeft(out var LeftIndex, out var LeftLeaf);
            NodePtr[Offset].GetRight(out var RightIndex, out var RightLeaf);
            int Parent = Offset + 1;

            if (LeftLeaf)
            {
                MortonParents[LeftIndex] = Parent;
            }
            else
            {
                NodePtr[LeftIndex].Parent = Parent;
            }

            if (RightLeaf)
            {
                MortonParents[RightIndex] = Parent;
            }
            else
            {
                NodePtr[RightIndex].Parent = Parent;
            }
        }

        private int GetNodeSplit(int i, int Range, int Dir, int PrefixNode)
        {
            int Split = 0;
            for (int t = Range / 2; t >= 1; t /= 2)
            {
                int PrefixTemp = GetPrefixLength(i, i + (Split + t) * Dir);
                if (PrefixTemp > PrefixNode)
                {
                    Split += t;
                }
            }
            return i + Split * Dir + Mathf.Min(Dir, 0);
        }

        private int GetNodeRange(int i, int Dir)
        {
            int MinPrefix = GetPrefixLength(i, i - Dir);
            int MaxRange = 8;
            // quick upper bound calc
            while (GetPrefixLength(i, i + Dir * MaxRange) > MinPrefix)
            {
                MaxRange *= 2;
            }
            // actual range with binary search
            int Range = 0;
            int T = MaxRange / 2;
            while (T >= 1)
            {
                if (GetPrefixLength(i, i + (Range + T) * Dir) > MinPrefix)
                {
                    Range += T;
                }
                T /= 2;
            }
            return Range;
        }

        private int GetDirection(int j)
        {
            return (int)Mathf.Sign(GetPrefixLength(j, j + 1) - GetPrefixLength(j, j - 1));
        }

        private int GetPrefixLength(int i, int j)
        {
            if (i < 0 || i >= MortonCodes.Length || j < 0 || j >= MortonCodes.Length)
                return -1;

            uint XOr;
            if (MortonCodes[i] != MortonCodes[j])
            {
                XOr = MortonCodes[i] ^ MortonCodes[j];
            }
            else
            {
                XOr = (uint)(i ^ j);
            }
            return IntSize - 1 - (int)Mathf.Log(XOr, 2);
        }

        private const int IntSize = 32;
    }

    [BurstCompile]
    /** Go bottom-up and calculate the bounding box of each node, consisting of its children*/
    public unsafe struct CalculateBB : IJobParallelFor
    {
        [ReadOnly]
        public NativeArray<uint> MortonCodes;

        [NativeDisableParallelForRestriction]
        public NativeArray<int> MortonParents;

        [NativeDisableParallelForRestriction]
        public NativeArray<Node> Nodes;

        [NativeDisableParallelForRestriction]
        public NativeArray<int> NodeChildren;


        public int ThreadCount;
        private int CountPerThread;
        [NativeDisableUnsafePtrRestriction]
        private Node* NodePtr;
        [NativeDisableUnsafePtrRestriction]
        private int* NodeChildrenPtr;

        public void Execute (int i)
        {
            // note: we are iterating over the morton codes, not the nodes!
            CountPerThread = MortonCodes.Length / ThreadCount;
            NodePtr = (Node*)Nodes.GetUnsafePtr();
            NodeChildrenPtr = (int*)NodeChildren.GetUnsafePtr();
            int LeftOverAmount = MortonCodes.Length - ThreadCount * CountPerThread;

            for (int j = 0; j < CountPerThread; j++)
            {
                Execute(i, j);
            }

            if (i >= LeftOverAmount)
                return;
            
            Execute(ThreadCount, i);
        }

        private void Execute(int ThreadIndex, int LoopIndex)
        {
            // since we start from the bottom, all initial children are morton codes
            int MortonChild = ThreadIndex * CountPerThread + LoopIndex;
            int ChildWithType = SetChildType(MortonChild, true);

            int Parent = MortonParents[MortonChild] - 1;
            while (Parent != -1)
            {
                // the first thread to reach immediately terminates, second will continue
                int OtherChildWithType = Interlocked.Exchange(ref NodeChildrenPtr[Parent], ChildWithType + 1) - 1;
                if (OtherChildWithType == -1)
                    return;

                GetChildVectors(ChildWithType, out var MinChild, out var MaxChild);
                GetChildVectors(OtherChildWithType, out var MinOtherChild, out var MaxOtherChild);
                NodePtr[Parent].Min = Vector3.Min(MinChild, MinOtherChild);
                NodePtr[Parent].Max = Vector3.Max(MaxChild, MaxOtherChild);

                // now it can only be a node parent
                ChildWithType = SetChildType(Parent, false);
                Parent = NodePtr[Parent].Parent - 1;
            }

        }

        private readonly int SetChildType(int Value, bool bIsMorton)
        {
            // can be simplified if we enforce the max amount 
            int TempValue = Value & (~(1 << ChildTypeIndex));
            TempValue = bIsMorton ?
                TempValue | (1 << ChildTypeIndex) :
                TempValue & (~(1 << ChildTypeIndex));
            return TempValue;
        }

        private readonly bool IsMortonChild(int Child, out int NewChild)
        {
            NewChild = Child & (~(1 << ChildTypeIndex));
            return (Child & (1 << ChildTypeIndex)) > 0;
        }

        private void GetChildVectors(int Child, out Vector3 Min, out Vector3 Max)
        {
            // if 2msb is 1 its a morton code child, otherwise a node child
            bool bIsChildMorton = IsMortonChild(Child, out Child);
            if (bIsChildMorton)
            {
                Morton.DeInterlace(MortonCodes[Child], out var Child1X, out var Child1Z);
                Min = new Vector3(Child1X, 0, Child1Z);
                Max = Min;
            }
            else
            {
                Min = NodePtr[Child].Min;
                Max = NodePtr[Child].Max;
            }
        }

        private const int ChildTypeIndex = 30;
    }

    /** Helper struct containg Node info*/
    public struct Node
    {
        public int First;
        public int Last;
        public int Split;
        // is 0 if no parent, as we cant init it with -1
        public int Parent;

        public Vector3 Min;
        public Vector3 Max;

        public readonly void GetLeft(out int Index, out bool bIsLeaf)
        {
            Index = Split;
            bIsLeaf = Mathf.Min(First, Last) == Split;
        }

        public readonly void GetRight(out int Index, out bool bIsLeaf)
        {
            Index = Split + 1;
            bIsLeaf = Mathf.Max(First, Last) == (Split + 1);
        }
    }
}
