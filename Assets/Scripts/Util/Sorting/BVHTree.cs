using System.Collections;
using System.Collections.Generic;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Collections;
using Unity.Jobs;
using UnityEngine;
using Unity.Jobs.LowLevel.Unsafe;
using System;
using UnityEngine.Assertions;

/*
 * New idea: build BVH each frame by
 * 1) computing morton code for locations (job), writing into shared native array - use membuffer
 * 2) parallel radix sort (job), write into new array
 * 
 * 3) split sorted array using paper, construct bvh
 * 4) calculate aabb 
 * NOTE: Use Jobs and SIMD for each step?
 * 
 * papers:
 * 1)
 *      https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
 *      https://snorrwe.onrender.com/posts/morton-table/
 * 
 * 2) 
 *      https://gpuopen.com/download/publications/Introduction_to_GPU_Radix_Sort.pdf
 *      https://stanford.edu/~rezab/classes/cme323/S16/projects_reports/he.pdf
 *      
 * 3)   
 *      https://graphics.cs.kuleuven.be/publications/BLD13OCCSVO/BLD13OCCSVO_paper.pdf
 *      https://developer.nvidia.com/blog/thinking-parallel-part-iii-tree-construction-gpu/
 *      https://developer.nvidia.com/blog/parallelforall/wp-content/uploads/2012/11/karras2012hpg_paper.pdf
 *      
 *  4)
 *      
 *      
 *  Jobs:
 *      base https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-creating-jobs.html
 *      parallel https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-parallel-for-jobs.html
 *      custom container https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-custom-nativecontainer-example.html
 *      https://docs.unity3d.com/6000.1/Documentation/ScriptReference/Jobs.IJobParallelForTransform.html
 *      
 *  SIMD:
 *  https://xoofx.github.io/blog/2023/07/09/10x-performance-with-simd-in-csharp-dotnet/
 *  #https://learn.microsoft.com/en-us/dotnet/standard/simd
 *  https://stackoverflow.com/questions/66605902/how-to-copy-from-an-array-to-a-vector256-and-vice-versa-based-on-the-array-index
 *  https://en.algorithmica.org/hpc/algorithms/prefix/
 *  
 *  prefix sum impl
 *  https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda
 *  https://www.cs.cmu.edu/~guyb/papers/Ble93.pdf
 */

public unsafe class BVHTree : ECSSystem
{
    private NativeArray<int> Counts;            // within a thread
    private NativeArray<int> PrefixCounts;      // within a bucket
    private NativeArray<int> GlobalPrefixCounts;      // overall 
    private NativeArray<TransformComponent> Components;
    private NativeArray<uint> MortonCodes;
    private NativeArray<uint> Target;
    private NativeArray<byte> Status;
    private JobHandle Handle;
    private AtomicSafetyHandle AtomicSafetyHandle;
    private bool bIsActive;
    private Dictionary<ComponentGroupIdentifier, Tuple<byte*[], List<int>>> RegisteredGroups = new();

    public void Init() 
    {
        Counts = new(ThreadCount * BucketSize * 3 + BucketSize, Allocator.Persistent);
        PrefixCounts = new(ThreadCount * BucketSize * 3 + BucketSize, Allocator.Persistent);
        GlobalPrefixCounts = new(BucketSize, Allocator.Persistent);

        MortonCodes = new(ECS.MaxEntities, Allocator.Persistent);
        Target = new(ECS.MaxEntities, Allocator.Persistent);
        Status = new(ThreadCount, Allocator.Persistent);

        //fill rest with default
        Handle = default;
        Components = default;
        AtomicSafetyHandle = default;
        bIsActive = false;
    }

    public bool IsBusy()
    {
        if (!Handle.IsCompleted)
            return true;

        foreach (var Value in Status)
        {
            if (Value < GetMaxIteration() - 1)
                return true;
        }
        return false;
    }

    public void Run()
    {
        if (RegisteredGroups.Count == 0 || bIsActive)
            return;

        int Start = 0;
        ClearArray(Status);
        Handle = ComputeMortonCodes(Start);

        for (int i = Start; i < GetMaxIteration(); i++)  
        {
            Handle = CreateRadixCountJob(i, Handle);
            Handle = CreateRadixPrefixSumJob(i, Handle);
            Handle = CreateRadixMoveJob(i, Handle);
        }
        bIsActive = true;
    }

    public void Register(ComponentGroupIdentifier Group, byte*[] ComponentPtrs, List<int> Counts)
    {
        if (!RegisteredGroups.ContainsKey(Group))
        {
            RegisteredGroups.Add(Group, null);
        }
        RegisteredGroups[Group] = new(ComponentPtrs, Counts);
    }
    
    private JobHandle ComputeMortonCodes(int Index)
    {
        int i = 0;
        foreach (var Entry in RegisteredGroups.Values) {
            Assert.IsTrue(i == 0, "Make this actually loopable");
            Components = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>((TransformComponent*)Entry.Key[0], Entry.Value[0], Allocator.None);
            AtomicSafetyHandle = AtomicSafetyHandle.Create();
            NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Components, AtomicSafetyHandle);
            i++;
        }

        bool bIsEven = (Index % 2) == 0;
        MortonSIMDJob Job = new()
        {
            Components = Components,
            MortonCodes = bIsEven ? MortonCodes : Target,
            WorkerCount = ThreadCount,
        };

        return Job.Schedule(ThreadCount, 1);
    }

    private JobHandle CreateRadixCountJob(int Index, JobHandle Dependency = default)
    {
        bool bIsEven = (Index % 2) == 0;
        GetVars(Index, out var Mask, out var LSBIndex);
        RadixSort.Count Job = new()
        {
            Counts = Counts,
            MortonCodes = bIsEven ? MortonCodes : Target,
            Target = bIsEven ? Target : MortonCodes,
            LSBIndex = LSBIndex,
            Mask = Mask,
            ThreadCount = ThreadCount
        };
        return Job.Schedule(ThreadCount, 1, Dependency);
    }

    private JobHandle CreateRadixPrefixSumJob(int Index, JobHandle Dependency = default)
    {
        bool bIsEven = (Index % 2) == 0;
        RadixSort.PrefixCount Job = new()
        {
            Target = bIsEven ? Target : MortonCodes,
            Counts = Counts,
            PrefixCounts = PrefixCounts,
            BucketSize = BucketSize,
            GlobalPrefixCounts = GlobalPrefixCounts
        };
        return Job.Schedule(Dependency);
    }

    private JobHandle CreateRadixMoveJob(int Index, JobHandle Dependency = default)
    {
        bool bIsEven = (Index % 2) == 0;
        GetVars(Index, out var Mask, out var LSBIndex);
        RadixSort.Move Job = new()
        {
            PrefixCounts = PrefixCounts,
            GlobalPrefixCounts = GlobalPrefixCounts,
            MortonCodes = bIsEven ? MortonCodes : Target,
            Target = bIsEven ? Target : MortonCodes,

            LSBIndex = LSBIndex,
            Mask = Mask,
            BucketSize = BucketSize,
            ThreadCount = ThreadCount,
            Counts = Counts,
            Iteration = Index,
            Status = Status
        };
        return Job.Schedule(ThreadCount, 1, Dependency);
    }

    private void GetVars(int i, out uint Mask, out int LSBIndex)
    {
        Mask = 0;
        LSBIndex = i * LSBWidth;
        for (int j = LSBIndex; j < LSBIndex + LSBWidth; j++)
        {
            Mask |= 1u << j;
        }

    }

    unsafe void ClearArray<T>(NativeArray<T> ToClear) where T : struct
    {
        UnsafeUtility.MemClear(
            NativeArrayUnsafeUtility.GetUnsafeBufferPointerWithoutChecks(ToClear),
            UnsafeUtility.SizeOf<T>() * ToClear.Length
        );
    }

    public void Destroy()
    {
        if (!Handle.IsCompleted)
        {
            Handle.Complete();
        }
        Counts.Dispose();
        PrefixCounts.Dispose();
        GlobalPrefixCounts.Dispose();
        MortonCodes.Dispose();
        Target.Dispose();
        Status.Dispose();
    }

    public void StartSystem()
    {
        Init();
    }

    public virtual void FixedTick(float FixedDelta) {
        Run();
    }

    private int GetMaxIteration()
    {
        return MortonWidth / LSBWidth;
    }

    public virtual void Tick(float Delta) {
        if (!bIsActive)
            return;

        if (IsBusy())
            return;

        Handle.Complete();
        // dont need to dispose components, as its a non-alloc call
        AtomicSafetyHandle.CheckDeallocateAndThrow(AtomicSafetyHandle);
        AtomicSafetyHandle.Release(AtomicSafetyHandle);

        //Validate();
        ClearArray(MortonCodes);
        ClearArray(Target);
        ClearArray(Counts);
        ClearArray(PrefixCounts);
        ClearArray(GlobalPrefixCounts);
        Handle = default;
        bIsActive = false;
    }

    private void Validate()
    {
        for (int i = 1; i < MortonCodes.Length; i++)
        {
            if (MortonCodes[i] >= MortonCodes[i - 1])
                continue;

            Debug.Log("Found wrong at " + i + ": " + MortonCodes[i - 1] + " !< " + MortonCodes[i]);
        }
    }

    private const int LSBWidth = 2;
    private const int MortonWidth = 32;
    private const int BucketSize = 4;
    private static int ThreadCount = JobsUtility.JobWorkerCount;
}
