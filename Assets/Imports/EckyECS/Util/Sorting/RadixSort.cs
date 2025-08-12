using log4net;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using Unity.Burst;
using Unity.Burst.Intrinsics;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Jobs;

/** 
 * Parallel Radix sort implementation
 * Used to sort the Morton codes (and therefor its 3d locations)
 * Since a MortonCode is 32Bit, we could reduce iterationcount by increasing the search bit width
 * For now, its 2 bit, aka 00 01 10 11
 */
public unsafe class RadixSort
{
    [BurstCompile(OptimizeFor = OptimizeFor.Performance)]
    public struct Count : IJobParallelFor
    {

        [ReadOnly]
        public NativeArray<uint> MortonCodes;

        public int LSBIndex;
        public uint Mask;
        public int ThreadCount;
        public int BucketSize;

        [NativeDisableParallelForRestriction]
        public NativeArray<int> Counts;
        [NativeDisableParallelForRestriction]
        public NativeArray<uint> Target;

        private int CountPerThread;

        /*
         * Actual execution of the radix sort. 
         * Used to be SIMD accelerated, but that made the algorithm way more complex 
         * cause the buckets needed to be shifted (making indexing terrible)
         */
        public void Execute(int i)
        {
            NativeArray<int> TempCounts = new(BucketSize, Allocator.Temp);
            CountPerThread = MortonCodes.Length / ThreadCount;
            int LeftOverCount = MortonCodes.Length - CountPerThread * ThreadCount;
            // can't split leftover between threads as it would swap positions of values
            // so the last thread will always run a tad longer, making any other thread wait for it :(
            bool bIsLeftOver = i == ThreadCount - 1;
            int Count = CountPerThread + (bIsLeftOver ? LeftOverCount : 0);

            for (int j = 0; j < Count; j++)
            {
                Execute(i, j, ref TempCounts);
            }
            
            NativeArray<int>.Copy(TempCounts, 0, Counts, i * BucketSize, BucketSize);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private void Execute(int i, int j, ref NativeArray<int> TempCounts)
        {
            int Offset = i * CountPerThread + j;
            uint MaskedCode = MortonCodes[Offset] & Mask;
            uint Value = MaskedCode >> LSBIndex;
            Target[Offset] = Value;
            TempCounts[(int)Value]++;
        }
    }

    [BurstCompile]
    /** Calculates the threaded prefix sum */
    public struct PrefixCount : IJob
    {
        [ReadOnly]
        public NativeArray<int> Counts;

        public NativeArray<int> PrefixCounts;
        public NativeArray<int> GlobalPrefixCounts;

        [NativeDisableParallelForRestriction]
        public NativeArray<uint> Target;

        public int BucketSize;

        public void Execute()
        {
            //can skip the first bucket
            for (int i = BucketSize; i < Counts.Length; i++)
            {
                PrefixCounts[i] = PrefixCounts[i - BucketSize] + Counts[i - BucketSize];
            }

            // first global prefix is always 0
            for (int i = 1; i < BucketSize; i++)
            {
                int Offset = PrefixCounts.Length - BucketSize + i - 1;
                GlobalPrefixCounts[i] = PrefixCounts[Offset] + Counts[Offset] + GlobalPrefixCounts[i - 1];
            }

            for (int i = 0; i < Target.Length; i++)
            {
                Target[i] = 0;
            }
        }
    }

    [BurstCompile]
    /** Sorts the elements internally according to their prefixes */
    public struct Move : IJobParallelFor
    {
        [ReadOnly]
        public NativeArray<uint> MortonCodes;
        [ReadOnly]
        public NativeArray<int> PrefixCounts;
        [ReadOnly]
        public NativeArray<int> GlobalPrefixCounts;
        [ReadOnly]
        public NativeArray<int> Counts;
        [ReadOnly]
        public NativeArray<int> IDIndices;

        public int LSBIndex;
        public uint Mask;
        public int ThreadCount;
        public int BucketSize;

        [NativeDisableParallelForRestriction]
        public NativeArray<uint> Target;
        [NativeDisableParallelForRestriction]
        public NativeArray<int> TargetIDIndices;

        public int Iteration;

        private int CountPerThread;
        public void Execute(int i)
        {

            NativeArray<int> LocalCounts = new(BucketSize, Allocator.Temp);
            CountPerThread = MortonCodes.Length / ThreadCount;
            int LeftOverCount = MortonCodes.Length - CountPerThread * ThreadCount;
            bool bIsLeftOver = i == ThreadCount - 1;
            int Count = CountPerThread + (bIsLeftOver ? LeftOverCount : 0);

            for (int j = 0; j < Count; j++)
            {
                Execute(i, j, ref LocalCounts);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private void Execute(int i, int j, ref NativeArray<int> LocalCounts)
        {
            int Offset = i * CountPerThread + j;
            uint MortonCode = MortonCodes[Offset];
            int Value = (int)((MortonCode & Mask) >> LSBIndex);
            int BucketIndex = BucketSize * i + Value;

            int TargetOffset = GlobalPrefixCounts[Value] + PrefixCounts[BucketIndex] + LocalCounts[Value];
            Target[TargetOffset] = MortonCode;
            TargetIDIndices[TargetOffset] = IDIndices[Offset];
            LocalCounts[Value]++;
        }
    }
}
