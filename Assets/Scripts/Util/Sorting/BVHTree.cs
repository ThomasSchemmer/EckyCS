using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Jobs;
using Unity.Jobs.LowLevel.Unsafe;
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
 * Uses flip-flop buffering for nodes and IDs 
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
public unsafe class BVHTree : ILocationProvider
{
    public TransformComponent* ComponentsPtr;
    public EntityID* IDsPtr;
    public int Count;

    // only Nodes, IDs and Components are persistent!
    private NativeArray<TransformComponent> Components;
    private NativeArray<EntityID> OriginalIDs;
    // but we need two nodes as we can only access the one we are currently
    // not writing to!
    private NativeArray<RadixTree.Node> NodesA;
    private NativeArray<RadixTree.Node> NodesB;
    private NativeArray<EntityID> FinalIDs;

    private NativeArray<uint> MortonCodes;
    private NativeArray<int> Counts;                // within a thread
    private NativeArray<int> PrefixCounts;          // within a bucket
    private NativeArray<int> GlobalPrefixCounts;    // overall 
    private NativeArray<int> MortonParents;
    private NativeArray<int> NodeChildren;
    private NativeArray<uint> Target;
    private NativeArray<EntityID> TargetIDsA;
    private NativeArray<EntityID> TargetIDsB;

    private JobHandle Handle;
    private AtomicSafetyHandle ComponentsSafetyHandle;
    private AtomicSafetyHandle IDsSafetyHandle;
    private bool bIsActive;
    private bool bWritesToNodesA;
    
    public void Init()
    {
        // uses flipflop bufers: nodes must be readable (which cannot be simultaneously accessed by jobs)
        NodesA = new(Count - 1, Allocator.Persistent);
        NodesB = new(Count - 1, Allocator.Persistent);
        FinalIDs = new(Count, Allocator.Persistent);

        // since we dont want to actually change the original components/IDs we use a ptr
        Components = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>(ComponentsPtr, Count, Allocator.None);
        ComponentsSafetyHandle = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Components, ComponentsSafetyHandle);

        OriginalIDs = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<EntityID>(IDsPtr, Count, Allocator.None);
        IDsSafetyHandle = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref OriginalIDs, IDsSafetyHandle);

        //fill rest with default
        bIsActive = false;
        bWritesToNodesA = true;
    }

    private void InitTemp()
    {
        MortonCodes = new(Count, Allocator.TempJob);

        Counts = new(ThreadCount * BucketSize + BucketSize, Allocator.TempJob);
        PrefixCounts = new(ThreadCount * BucketSize + BucketSize, Allocator.TempJob);
        GlobalPrefixCounts = new(BucketSize, Allocator.TempJob);

        Target = new(Count, Allocator.TempJob);
        TargetIDsA = new(Count, Allocator.TempJob);
        TargetIDsB = new(Count, Allocator.TempJob);
        MortonParents = new(MortonCodes.Length, Allocator.TempJob);
        NodeChildren = new(NodesA.Length, Allocator.TempJob);
    }

    public void Run()
    {
        if (ComponentsPtr == null || IDsPtr  == null || bIsActive || !Handle.IsCompleted)
            return;

        int Start = 0;
        InitTemp();
        ClearArray(GetWriteNodeArray());
        ClearArray(FinalIDs);
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

    public void Register(TransformComponent* ComponentsPtr, EntityID* IDsPtr, int Count)
    {
        this.ComponentsPtr = ComponentsPtr;
        this.IDsPtr = IDsPtr;
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
        bool bIsLast = Index == GetMaxIteration() - 1;
        bool bIsFirst = Index == 0;
        GetVars(Index, out var Mask, out var LSBIndex);
        RadixSort.Move Job = new()
        {
            PrefixCounts = PrefixCounts,
            GlobalPrefixCounts = GlobalPrefixCounts,
            MortonCodes = bIsEven ? MortonCodes : Target,
            Target = bIsEven ? Target : MortonCodes,
            IDs = bIsFirst ? OriginalIDs : (bIsEven ? TargetIDsA : TargetIDsB),
            TargetIDs = bIsLast ? FinalIDs : (bIsEven ? TargetIDsB : TargetIDsA),

            LSBIndex = LSBIndex,
            Mask = Mask,
            BucketSize = BucketSize,
            ThreadCount = ThreadCount,
            Counts = Counts,
            Iteration = Index,
        };
        return Job.Schedule(ThreadCount, 1, Dependency);
    }

    private JobHandle CreateRadixTreeCreateJob(JobHandle Dependency = default)
    {
        RadixTree.Create Job = new()
        {
            MortonCodes = MortonCodes,
            ThreadCount = ThreadCount,
            Nodes = GetWriteNodeArray(),
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
            Nodes = GetWriteNodeArray(),
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

    private NativeArray<RadixTree.Node> GetWriteNodeArray()
    {
        return bWritesToNodesA ? NodesA : NodesB;
    }

    private NativeArray<RadixTree.Node> GetReadNodeArray()
    {
        return bWritesToNodesA ? NodesB : NodesA;
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
        if (NodesA.IsCreated) NodesA.Dispose();
        if (NodesB.IsCreated) NodesB.Dispose();
        if (TargetIDsA.IsCreated) TargetIDsA.Dispose();
        if (TargetIDsB.IsCreated) TargetIDsB.Dispose();
        if (MortonParents.IsCreated) MortonParents.Dispose();
        if (FinalIDs.IsCreated) FinalIDs.Dispose();

        // dont need to dispose components / IDs, as they are non-alloc calls
        if (!AtomicSafetyHandle.IsDefaultValue(ComponentsSafetyHandle) && AtomicSafetyHandle.IsHandleValid(ComponentsSafetyHandle))
        {
            AtomicSafetyHandle.CheckDeallocateAndThrow(ComponentsSafetyHandle);
            AtomicSafetyHandle.Release(ComponentsSafetyHandle);
        }
        if (!AtomicSafetyHandle.IsDefaultValue(IDsSafetyHandle) && AtomicSafetyHandle.IsHandleValid(IDsSafetyHandle))
        {
            AtomicSafetyHandle.CheckDeallocateAndThrow(IDsSafetyHandle);
            AtomicSafetyHandle.Release(IDsSafetyHandle);
        }
    }

    private int GetMaxIteration()
    {
        return MortonWidth / LSBWidth;
    }

    public virtual void Tick(float Delta) {
        if (!bIsActive || !Handle.IsCompleted)
            return;

        Handle.Complete();

        //Validate();
        MortonCodes.Dispose();
        Counts.Dispose();
        PrefixCounts.Dispose();
        GlobalPrefixCounts.Dispose();
        MortonParents.Dispose();
        NodeChildren.Dispose();
        Target.Dispose();
        TargetIDsA.Dispose();
        TargetIDsB.Dispose();

        Handle = default;
        bIsActive = false;
        bWritesToNodesA = !bWritesToNodesA;
    }

    public void Debug()
    {
        /** Needs to be called from OnDrawGizmos() */
        Gizmos.color = Color.green;
        foreach (var Node in GetReadNodeArray())
        {
            Vector3 Center = (Node.Min + Node.Max) / 2f;
            Vector3 Range = Node.Max - Center;
            Gizmos.DrawWireCube(Center, Range);
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

    public List<EntityID> GetAllAt(Vector3 Position, float Range)
    {
        return null;
    }

    private const int LSBWidth = 2;
    private const int MortonWidth = 32;
    private const int BucketSize = 4;
    private readonly static int ThreadCount = JobsUtility.JobWorkerCount;
}
