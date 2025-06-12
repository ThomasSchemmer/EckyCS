using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Jobs;
using Unity.Jobs.LowLevel.Unsafe;
using Unity.VisualScripting.YamlDotNet.Core.Events;
using UnityEngine;

/*
 * Bounding Volum Hierarchy Tree, used per GroupId to quickly find Entities based on their location
 * See @LocationSystem for Usage
 * Will be constructed every FixedUpdate
 * Is heavily parallized with Unities Job system, uses @Morton, @RadixTree and @RadixSort
 * Overall algorithm:
 * 1) computing morton code for locations (job), writing into shared native array - use membuffer
 * 2) parallel radix sort on MortonCodes (job)
 * 3) create radix tree from sorted MortonCodes
 * 4) calculate aabb for each radix node from childrens, bottom-up
 * 
 * NOTE: Originally planned to use SIMD for even more performance, but the majority of the RadixTree can't be SIMD'd
 * as it writes into different, random locations. RadixSort could be, but makes it ALOT more complicated due to indexing
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
 * 3 & 4)   
 *      https://graphics.cs.kuleuven.be/publications/BLD13OCCSVO/BLD13OCCSVO_paper.pdf
 *      https://developer.nvidia.com/blog/thinking-parallel-part-iii-tree-construction-gpu/
 *      https://developer.nvidia.com/blog/parallelforall/wp-content/uploads/2012/11/karras2012hpg_paper.pdf
 *      
 *  Jobs:
 *      base https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-creating-jobs.html
 *      parallel https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-parallel-for-jobs.html
 *      custom container https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-custom-nativecontainer-example.html
 *      https://docs.unity3d.com/6000.1/Documentation/ScriptReference/Jobs.IJobParallelForTransform.html
 *      
 */

public unsafe class BVHTree
{
    public TransformComponent* ComponentsPtr;
    public int Count;

    // only Nodes and Components are persistent!
    private NativeArray<TransformComponent> Components;
    private NativeArray<RadixTree.Node> Nodes;

    private NativeArray<uint> MortonCodes;
    private NativeArray<int> Counts;                // within a thread
    private NativeArray<int> PrefixCounts;          // within a bucket
    private NativeArray<int> GlobalPrefixCounts;    // overall 
    private NativeArray<int> MortonParents;
    private NativeArray<int> NodeChildren;
    private NativeArray<uint> Target;
    private NativeArray<byte> Status;


    private JobHandle Handle;
    private AtomicSafetyHandle AtomicSafetyHandle;
    private bool bIsActive;
    
    public void Init()
    {
        // components will be a Ptr assignment
        Nodes = new(Count - 1, Allocator.Persistent);

        //fill rest with default
        Handle = default;
        Components = default;
        AtomicSafetyHandle = default;
        bIsActive = false;
    }

    private void InitTemp()
    {
        Components = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>(ComponentsPtr, Count, Allocator.None);
        AtomicSafetyHandle = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Components, AtomicSafetyHandle);

        MortonCodes = new(Count, Allocator.TempJob);

        Counts = new(ThreadCount * BucketSize + BucketSize, Allocator.TempJob);
        PrefixCounts = new(ThreadCount * BucketSize + BucketSize, Allocator.TempJob);
        GlobalPrefixCounts = new(BucketSize, Allocator.TempJob);

        Target = new(Count, Allocator.TempJob);
        Status = new(ThreadCount, Allocator.TempJob);
        MortonParents = new(MortonCodes.Length, Allocator.TempJob);
        NodeChildren = new(Nodes.Length, Allocator.TempJob);
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
        if (ComponentsPtr == null || bIsActive)
            return;

        int Start = 0;
        InitTemp();
        ClearArray(Status);
        ClearArray(Nodes);
        Handle = ComputeMortonCodes(Start);

        for (int i = Start; i < GetMaxIteration(); i++)  
        {
            Handle = CreateRadixCountJob(i, Handle);
            Handle = CreateRadixPrefixSumJob(i, Handle);
            Handle = CreateRadixMoveJob(i, Handle);
        }
        Handle = CreateRadixTreeCreateJob(Handle);
        Handle = CreateRadixTreeBBJob(Handle);
        bIsActive = true;
    }

    public void Register(TransformComponent* Ptr, int Count)
    {
        ComponentsPtr = Ptr;
        if (this.Count != Count)
        {
            this.Count = Count;
            Destroy();
            Init();
        }
    }
    
    private JobHandle ComputeMortonCodes(int Index)
    {
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
            ThreadCount = ThreadCount,
            BucketSize = BucketSize
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

    private JobHandle CreateRadixTreeCreateJob(JobHandle Dependency = default)
    {
        RadixTree.Create Job = new()
        {
            MortonCodes = MortonCodes,
            ThreadCount = ThreadCount,
            Nodes = Nodes,
            MortonParents = MortonParents
        };
        return Job.Schedule(ThreadCount, 1, Dependency);
    }

    private JobHandle CreateRadixTreeBBJob(JobHandle Dependency = default)
    {
        RadixTree.CalculateBB Job = new()
        {
            MortonCodes = MortonCodes,
            MortonParents = MortonParents,
            Nodes = Nodes,
            NodeChildren = NodeChildren,
            ThreadCount = ThreadCount,
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

    private unsafe void ClearArray<T>(NativeArray<T> ToClear) where T : struct
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

        if (Counts.IsCreated) Counts.Dispose();
        if (PrefixCounts.IsCreated) PrefixCounts.Dispose();
        if (GlobalPrefixCounts.IsCreated) GlobalPrefixCounts.Dispose();
        if (MortonCodes.IsCreated) MortonCodes.Dispose();
        if (Target.IsCreated) Target.Dispose();
        if (Status.IsCreated) Status.Dispose();
        if (Nodes.IsCreated) Nodes.Dispose();
        if (MortonParents.IsCreated) MortonParents.Dispose();
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
        MortonCodes.Dispose();
        Counts.Dispose();
        PrefixCounts.Dispose();
        GlobalPrefixCounts.Dispose();
        MortonParents.Dispose();
        NodeChildren.Dispose();
        Target.Dispose();
        Status.Dispose();

        Handle = default;
        bIsActive = false;
    }

    /** Needs to be called from OnDrawGizmos() */
    public void Debug()
    {
        if (IsBusy())
            return;

        foreach (var Node in Nodes)
        {
            int Size = (int)Mathf.Abs(Node.First - Node.Last);
            int Width = (int)(Size / (float)Count * 15);
            Vector3 Center = (Node.Min + Node.Max) / 2f;
            Vector3 Range = Node.Max - Center;
            Gizmos.DrawCube(Center, Range);
        }
    }

    private void Validate()
    {
        for (int i = 1; i < MortonCodes.Length; i++)
        {
            if (MortonCodes[i] >= MortonCodes[i - 1])
                continue;

            UnityEngine.Debug.Log("Found wrong at " + i + ": " + MortonCodes[i - 1] + " !< " + MortonCodes[i]);
        }
    }

    private const int LSBWidth = 2;
    private const int MortonWidth = 32;
    private const int BucketSize = 4;
    private static int ThreadCount = JobsUtility.JobWorkerCount;
}
