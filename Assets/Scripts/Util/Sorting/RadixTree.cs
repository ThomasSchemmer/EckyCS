using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Jobs;
using UnityEngine;

/** 
 * Helper struct to create a radix tree from an already sorted array of morton codes
 *
 * implements https://developer.nvidia.com/blog/parallelforall/wp-content/uploads/2012/11/karras2012hpg_paper.pdf
 */
public struct RadixTree
{
    public struct Create : IJobParallelFor
    {
        [ReadOnly]
        public NativeArray<int> MortonCodes;

        [NativeDisableParallelForRestriction]
        public NativeArray<Node> Nodes;

        public int ThreadCount;
        private int CountPerThread;

        public void Execute(int i)
        {
            CountPerThread = (MortonCodes.Length - 1) / ThreadCount;
            for (int j = 0; j < CountPerThread; j++)
            {
                Execute(i, j);
            }
        }

        private void Execute(int i, int j)
        {
            int Offset = i * ThreadCount + j;
            int Dir = GetDirection(Offset);
            int End = GetNodeEnd(Offset, Dir);
            int PrefixNode = GetPrefixLength(Offset, End);
            
        }

        private int GetNodeEnd(int i, int Dir)
        {
            int MinPrefix = GetPrefixLength(i, i - Dir);
            int LMax = 8;
            // quick upper bound calc
            while (GetPrefixLength(i, i + Dir * LMax) > MinPrefix)
            {
                LMax *= 2;
            }
            // actual range with binary search
            int L = 0;
            int T = LMax / 2;
            while (T >= 1)
            {
                if (GetPrefixLength(i, i + (L + T) * Dir) > MinPrefix)
                {
                    L += T;
                }
                T /= 2;
            }
            return i + L * Dir;
        }

        private int GetDirection(int j)
        {
            return (int)Mathf.Sign(GetPrefixLength(j, j + 1) - GetPrefixLength(j, j - 1));
        }

        private int GetPrefixLength(int i, int j)
        {
            if (i < 0 || i >= Nodes.Length || j < 0 || j >= Nodes.Length)
                return -1;

            for(int Bit = 0; Bit < IntSize; Bit++) {
                int Mask = 1 << Bit;
                if ((MortonCodes[i] & Mask) != (MortonCodes[j] & Mask))
                    return Bit;
            }
            return IntSize;
        }

        private const int IntSize = 32;
    }

    public struct Node
    {
        public int Index;
        public int First;
        public int Last;
        public int Split;
    }
}
