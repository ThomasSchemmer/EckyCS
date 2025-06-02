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
using static Unity.Burst.Intrinsics.X86.Avx;
using static Unity.Burst.Intrinsics.X86.Avx2;

/** 
 * Parallel Radix sort implementation
 * Used to sort the Morton codes (and therefor its 3d locations)
 * Since a MortonCode is 32Bit, we could reduce iterationcount by increasing the search bit width
 * For now, its 2 bit, aka 00 01 10 11
 */
public unsafe class RadixSort
{
    [BurstCompile(OptimizeFor = OptimizeFor.Performance)]
    /** SIMD accelerated counting of each element (per 2 bits) */
    public struct Count : IJobParallelFor
    {

        [ReadOnly]
        public NativeArray<uint> MortonCodes;

        public int LSBIndex;
        public uint Mask;
        public int ThreadCount;

        [NativeDisableParallelForRestriction]
        public NativeArray<int> Counts;
        [NativeDisableParallelForRestriction]
        public NativeArray<uint> Target;

        private Size Size;

        /*
         * Actual execution of the radix sort. 
         * has 3 loops, for unrolled vectorized, vectorized and regular to cram as much performance
         * out of the counting as possible
         */
        public void Execute(int i)
        {

            bool bIsLeftOver = i == ThreadCount - 1;
            NativeArray<int> TempCounts = new(bIsLeftOver ? LeftOverCountSize : RegularCountSize, Allocator.Temp);
            v256* vMortons = ((v256*)MortonCodes.GetUnsafeReadOnlyPtr());
            v256* vTargets = ((v256*)Target.GetUnsafeReadOnlyPtr());
            var vMask = new v256(Mask);
            var v0 = mm256_set1_epi8(0);
            var v1 = mm256_set1_epi8(1);
            var v2 = mm256_set1_epi8(2);
            var v3 = mm256_set1_epi8(3);

            Size.Init(MortonCodes.Length, ThreadCount);

            // unrolled SIMD loop (32 times faster)
            for (int j = 0; j < Size.UnrolledSIMDCount; j++)
            {
                int Offset = Size.GetUnrolledOffset(i, j);
                var t0 = mm256_and_ps(*(vMortons + Offset + 00), vMask); // x & mask
                var t1 = mm256_and_ps(*(vMortons + Offset + 01), vMask);
                var t2 = mm256_and_ps(*(vMortons + Offset + 02), vMask);
                var t3 = mm256_and_ps(*(vMortons + Offset + 03), vMask);

                var t01 = mm256_srlv_epi32(t0, new(LSBIndex)); // x >> Index
                var t11 = mm256_srlv_epi32(t1, new(LSBIndex));
                var t21 = mm256_srlv_epi32(t2, new(LSBIndex));
                var t31 = mm256_srlv_epi32(t3, new(LSBIndex));

                mm256_storeu_si256(vTargets + Offset + 00, t01);
                mm256_storeu_si256(vTargets + Offset + 01, t11);
                mm256_storeu_si256(vTargets + Offset + 02, t21);
                mm256_storeu_si256(vTargets + Offset + 03, t31);

                // since each value must be < 8 we can squeeze in each value into a byte
                // combining 4 into an integer
                var t02 = mm256_sllv_epi32(t01, new(00)); // x << 0
                var t12 = mm256_sllv_epi32(t11, new(08));
                var t22 = mm256_sllv_epi32(t21, new(16));
                var t32 = mm256_sllv_epi32(t31, new(24));

                var combined =                            // x3 | x2 | x1 | x0
                    mm256_or_si256(t02,
                    mm256_or_si256(t12,
                    mm256_or_si256(t22, t32))
                );

                var v0Combined = mm256_cmpeq_epi8(combined, v0);   // x == 0 ? -INT_MAX : 0
                var v0Masked = mm256_and_si256(v0Combined, mm256_set1_epi8(1)); // x & 0x01 (bytewise!)
                var v1Combined = mm256_cmpeq_epi8(combined, v1);
                var v1Masked = mm256_and_si256(v1Combined, mm256_set1_epi8(1));
                var v2Combined = mm256_cmpeq_epi8(combined, v2);
                var v2Masked = mm256_and_si256(v2Combined, mm256_set1_epi8(1));
                var v3Combined = mm256_cmpeq_epi8(combined, v3);
                var v3Masked = mm256_and_si256(v3Combined, mm256_set1_epi8(1));

                // now iterate over each byte and count (mm256_sad hack)
                var v0result = mm256_sad_epu8(v0Masked, mm256_setzero_si256()); // count per 8 bytes (in each long)
                TempCounts[0] += (int)(v0result.SLong0 + v0result.SLong1 + v0result.SLong2 + v0result.SLong3);
                var v1result = mm256_sad_epu8(v1Masked, mm256_setzero_si256());
                TempCounts[1] += (int)(v1result.SLong0 + v1result.SLong1 + v1result.SLong2 + v1result.SLong3);
                var v2result = mm256_sad_epu8(v2Masked, mm256_setzero_si256());
                TempCounts[2] += (int)(v2result.SLong0 + v2result.SLong1 + v2result.SLong2 + v2result.SLong3);
                var v3result = mm256_sad_epu8(v3Masked, mm256_setzero_si256());
                TempCounts[3] += (int)(v3result.SLong0 + v3result.SLong1 + v3result.SLong2 + v3result.SLong3);

            }

            // same loop as before, just not unrolled (so only 8 times faster)
            for (int j = 0; j < Size.SIMDCount; j++)
            {

                var Offset = Size.GetSIMDOffset(i, j);
                var t0 = mm256_and_ps(*(vMortons + Offset), vMask); // x & mask
                var t01 = mm256_srlv_epi32(t0, new(LSBIndex)); // x >> Index

                mm256_storeu_si256(vTargets + Offset + 00, t01);

                // same concept as unrolled code, but this time 32 bit (only one entry)
                var v0Combined = mm256_cmpeq_epi32(t01, mm256_set1_epi32(0));   // x == 0 ? -INT_MAX : 0
                var v0Masked = mm256_and_si256(v0Combined, mm256_set1_epi32(1)); // x & 0x01 (bytewise!)
                var v1Combined = mm256_cmpeq_epi32(t01, mm256_set1_epi32(1));
                var v1Masked = mm256_and_si256(v1Combined, mm256_set1_epi32(1));
                var v2Combined = mm256_cmpeq_epi32(t01, mm256_set1_epi32(2));
                var v2Masked = mm256_and_si256(v2Combined, mm256_set1_epi32(1));
                var v3Combined = mm256_cmpeq_epi32(t01, mm256_set1_epi32(3));
                var v3Masked = mm256_and_si256(v3Combined, mm256_set1_epi32(1));

                // now iterate over each byte and count (mm256_sad hack)
                // sad is only available in epu8, but since it contains a counter its fine
                var v0result = mm256_sad_epu8(v0Masked, mm256_setzero_si256()); // count per 8 bytes (in each long)
                TempCounts[4] += (int)(v0result.SLong0 + v0result.SLong1 + v0result.SLong2 + v0result.SLong3);
                var v1result = mm256_sad_epu8(v1Masked, mm256_setzero_si256());
                TempCounts[5] += (int)(v1result.SLong0 + v1result.SLong1 + v1result.SLong2 + v1result.SLong3);
                var v2result = mm256_sad_epu8(v2Masked, mm256_setzero_si256());
                TempCounts[6] += (int)(v2result.SLong0 + v2result.SLong1 + v2result.SLong2 + v2result.SLong3);
                var v3result = mm256_sad_epu8(v3Masked, mm256_setzero_si256());
                TempCounts[7] += (int)(v3result.SLong0 + v3result.SLong1 + v3result.SLong2 + v3result.SLong3);

            }

            // standard loop, neither unrolled nor vectorized
            for (int j = 0; j < Size.StandardCount; j++)
            {
                var Offset = Size.GetStandardOffset(i, j);
                Execute(Offset, false, ref TempCounts);
            }

            // Remainder of split can only really be handled by one thread, should be < ThreadCount
            if (bIsLeftOver)
            {
                for (int j = 0; j < Size.LeftOverCount; j++)
                {
                    var Offset = Size.GetLeftOverOffset(j);
                    Execute(Offset, true, ref TempCounts);
                }
            }
            
            // the algorithm only works if the buckets are continuous - due to SIMD we garuantee the opposite
            // so we need to split each bucket according to its position, aka have its own count value
            NativeArray<int>.Copy(TempCounts, 0, Counts, i * 12, bIsLeftOver ? LeftOverCountSize : RegularCountSize);
        }

        private void Execute(int Offset, bool bIsLeftOver, ref NativeArray<int> TempCounts)
        {
            uint MaskedCode = MortonCodes[Offset] & Mask;
            uint Value = MaskedCode >> LSBIndex;
            Target[Offset] = Value;
            int TargetOffset = (int)Value + 8 + (bIsLeftOver ? 4 : 0);
            TempCounts[TargetOffset]++;
        }

        private const int RegularCountSize = 12;
        private const int LeftOverCountSize = 16;
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

        [NativeDisableContainerSafetyRestriction]
        [NativeDisableParallelForRestriction]
        public NativeArray<byte> Status;

        public int LSBIndex;
        public uint Mask;
        public int ThreadCount;
        public int BucketSize;

        [NativeDisableParallelForRestriction]
        public NativeArray<uint> Target;

        public int Iteration;

        private Size Size;

        public void Execute(int i)
        {

            NativeArray<int> LocalCounts = new(BucketSize * 3, Allocator.Temp);
            Size.Init(MortonCodes.Length, ThreadCount);

            for (int j = 0; j < Size.CountPerThread; j++)
            {
                Execute(i, j, false, ref LocalCounts);
            }

            for (int j = 0; j < Size.GetLeftOverAmount(i); j++) {
                Execute(i, j, true, ref LocalCounts);
            }

            Status[i] = (byte)Iteration;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private void Execute(int i, int j, bool bIsLeftOver, ref NativeArray<int> LocalCounts)
        {
            Size.LoopToOffset(i, j, out var Offset, out var BucketIndex);
            Offset = bIsLeftOver ? Size.GetLeftOverOffset(j) : Offset;

            uint MortonCode = MortonCodes[Offset];
            int Value = (int)((MortonCode & Mask) >> LSBIndex);

            int FinalBucketIndex = BucketSize * i * 3 + BucketIndex * BucketSize + Value;

            int TargetOffset = GlobalPrefixCounts[Value] + PrefixCounts[FinalBucketIndex] + LocalCounts[Value + BucketIndex * BucketSize];
            Target[TargetOffset] = Iteration == 55 ? (uint)(Target[TargetOffset] * 10 + i + 1) : MortonCode;
            LocalCounts[Value + BucketIndex * BucketSize]++;
        }
    }

    /** Helper struct to share size computations */
    private struct Size
    {
        public int CountPerThread;
        public int UnrolledSIMDCount;
        public int SIMDCount;
        public int StandardCount;
        public int LeftOverCount;

        // dont work on continuous memory block per thread, but all Unrolled, SIMD and standard ones
        // are made on distinct memory blocks - makes indexing v256 easier
        // eg second thread would start at 50, so v256 would start at 2 (48 is closest / 8)
        public int UnrolledStart;
        public int SIMDStart;
        public int StandardStart;
        public int LeftOverStart;

        public const int UnrolledSIMDSize = 32;
        public const int SIMDSize = 8;

        private int ThreadCount;

        public void Init(int Length, int ThreadCount)
        {
            this.ThreadCount = ThreadCount;
            CountPerThread = Length / ThreadCount;

            // calculate how many of each iterations we can fit 
            UnrolledSIMDCount = CountPerThread / UnrolledSIMDSize;
            SIMDCount = (CountPerThread - UnrolledSIMDCount * UnrolledSIMDSize) / SIMDSize;
            StandardCount = CountPerThread - UnrolledSIMDCount * UnrolledSIMDSize - SIMDCount * SIMDSize;
            LeftOverCount = Length - CountPerThread * ThreadCount;

            UnrolledStart = 0;
            SIMDStart = UnrolledSIMDCount * UnrolledSIMDSize * ThreadCount;
            StandardStart = SIMDStart + SIMDCount * SIMDSize * ThreadCount;
            LeftOverStart = CountPerThread * ThreadCount;

        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly void LoopToOffset(int ThreadIndex, int LoopIndex, out int Offset, out int BucketIndex)
        {
            Offset = -1;
            BucketIndex = -1;
            if (LoopIndex < UnrolledSIMDCount * UnrolledSIMDSize)
            {
                Offset = UnrolledStart + UnrolledSIMDCount * UnrolledSIMDSize * ThreadIndex + LoopIndex;
                BucketIndex = 0;
                return;
            }

            LoopIndex -= UnrolledSIMDCount * UnrolledSIMDSize;
            if (LoopIndex < SIMDCount * SIMDSize)
            {
                Offset = SIMDStart + SIMDCount * SIMDSize * ThreadIndex + LoopIndex;
                BucketIndex = 1;
                return;
            }

            LoopIndex -= SIMDCount * SIMDSize;
            Offset = StandardStart + StandardCount * ThreadIndex + LoopIndex;
            BucketIndex = 2;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly int GetUnrolledOffset(int ThreadIndex, int LoopIndex)
        {
            return (UnrolledStart + ThreadIndex * UnrolledSIMDCount * UnrolledSIMDSize + LoopIndex * UnrolledSIMDSize) / SIMDSize;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly int GetSIMDOffset(int ThreadIndex, int LoopIndex)
        {
            return (SIMDStart + ThreadIndex * SIMDCount * SIMDSize + LoopIndex * SIMDSize) / SIMDSize;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly int GetStandardOffset(int ThreadIndex, int LoopIndex)
        {
            return StandardStart + ThreadIndex * StandardCount + LoopIndex;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly int GetLeftOverOffset(int LoopIndex)
        {
            return LeftOverStart + LoopIndex;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly int GetLeftOverAmount(int ThreadIndex)
        {
            return ThreadIndex == ThreadCount - 1 ? LeftOverCount : 0;
        }
    }
}
