using log4net.Util;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
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
public unsafe class BVHTree 
{
    public TransformComponent* ComponentsPtr;
    public EntityID* IDsPtr;
    public int Count;

    // only FinalNodes, (Original/Final)IDs and Components are persistent!
    private NativeArray<TransformComponent> Components;
    private NativeArray<EntityID> OriginalIDs;
    private NativeArray<int> FinalIDIndices;
    private NativeArray<RadixTree.Node> FinalNodes;

    private NativeArray<int> Counts;                // within a thread
    private NativeArray<int> PrefixCounts;          // within a bucket
    private NativeArray<int> GlobalPrefixCounts;    // overall 
    private NativeArray<int> MortonParents;
    private NativeArray<int> NodeChildren;
    // the following native arrays are flipflopping
    private NativeArray<uint> MortonCodesA;
    private NativeArray<uint> MortonCodesB;
    private NativeArray<int> TargetIDIndicesA;
    private NativeArray<int> TargetIDIndicesB;
    private NativeArray<RadixTree.Node> Nodes;

    private JobHandle Handle;
    private AtomicSafetyHandle ComponentsSafetyHandle;
    private AtomicSafetyHandle IDsSafetyHandle;
    private bool bIsActive;
    private bool bWritesToNodesA;
    
    public void Init()
    {
        FinalIDIndices = new(Count, Allocator.Persistent);
        FinalNodes = new(Count - 1, Allocator.Persistent);

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
        MortonCodesA = new(Count, Allocator.TempJob);

        Nodes = new(Count - 1, Allocator.TempJob);

        Counts = new(ThreadCount * BucketSize + BucketSize, Allocator.TempJob);
        PrefixCounts = new(ThreadCount * BucketSize + BucketSize, Allocator.TempJob);
        GlobalPrefixCounts = new(BucketSize, Allocator.TempJob);

        MortonCodesB = new(Count, Allocator.TempJob);
        // initialize with "standard" distribution
        TargetIDIndicesA = new(Count, Allocator.TempJob);
        for (int i = 0; i < TargetIDIndicesA.Length; i++)
        {
            TargetIDIndicesA[i] = i;
        }
        TargetIDIndicesB = new(Count, Allocator.TempJob);
        MortonParents = new(MortonCodesA.Length, Allocator.TempJob);
        NodeChildren = new(Nodes.Length, Allocator.TempJob);
    }

    public void Run()
    {
        if (ComponentsPtr == null || IDsPtr  == null || bIsActive || !Handle.IsCompleted)
            return;

        int Start = 0;
        InitTemp();
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
            MortonCodes = bIsEven ? MortonCodesA : MortonCodesB,
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
            MortonCodes = bIsEven ? MortonCodesA : MortonCodesB,
            Target = bIsEven ? MortonCodesB : MortonCodesA,
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
            Target = bIsEven ? MortonCodesB : MortonCodesA,
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
            MortonCodes = bIsEven ? MortonCodesA : MortonCodesB,
            Target = bIsEven ? MortonCodesB : MortonCodesA,
            IDIndices = bIsEven ? TargetIDIndicesA : TargetIDIndicesB,
            TargetIDIndices = bIsEven ? TargetIDIndicesB : TargetIDIndicesA,

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
            MortonCodes = MortonCodesA,
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
            MortonCodes = MortonCodesA,
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
        if (!Handle.IsCompleted || bIsActive)
        {
            Handle.Complete();
        }

        if (Counts.IsCreated) Counts.Dispose();
        if (PrefixCounts.IsCreated) PrefixCounts.Dispose();
        if (GlobalPrefixCounts.IsCreated) GlobalPrefixCounts.Dispose();
        if (MortonCodesA.IsCreated) MortonCodesA.Dispose();
        if (MortonCodesB.IsCreated) MortonCodesB.Dispose();
        if (Nodes.IsCreated) Nodes.Dispose();
        if (TargetIDIndicesA.IsCreated) TargetIDIndicesA.Dispose();
        if (TargetIDIndicesB.IsCreated) TargetIDIndicesB.Dispose();
        if (MortonParents.IsCreated) MortonParents.Dispose();
        if (FinalIDIndices.IsCreated) FinalIDIndices.Dispose();
        if (FinalNodes.IsCreated) FinalNodes.Dispose();

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

        FinalIDIndices.CopyFrom(TargetIDIndicesA);
        FinalNodes.CopyFrom(Nodes);

        //Validate();
        Counts.Dispose();
        PrefixCounts.Dispose();
        GlobalPrefixCounts.Dispose();
        MortonParents.Dispose();
        NodeChildren.Dispose();
        MortonCodesA.Dispose();
        MortonCodesB.Dispose();
        TargetIDIndicesA.Dispose();
        TargetIDIndicesB.Dispose();
        Nodes.Dispose();

        Handle = default;
        bIsActive = false;
        bWritesToNodesA = !bWritesToNodesA;

    }

    public void Debug()
    {
        /** Needs to be called from OnDrawGizmos() */
        Gizmos.color = Color.green;
        foreach (var Node in Nodes)
        {
            Vector3 Center = (Node.Min + Node.Max) / 2f;
            Vector3 Range = Node.Max - Center;
            Gizmos.DrawWireCube(Center, Range);
        }
    }

    private void Validate()
    {
        for (int i = 1; i < MortonCodesA.Length; i++)
        {
            if (MortonCodesA[i] >= MortonCodesA[i - 1])
                continue;

            UnityEngine.Debug.Log("Found wrong at " + i + ": " + MortonCodesA[i - 1] + " !< " + MortonCodesA[i]);
        }
    }

    public unsafe List<EntityID> GetAllAt(Rect Target)
    {
        if (FinalNodes.Length == 0)
        {
            var Temp = new Vector2(Components[0].PosX, Components[0].PosZ);
            if (Target.Contains(Temp)) 
                return new List<EntityID>() { OriginalIDs[FinalIDIndices[0]] };
            else 
                return new();
        }

        return GetAllAtRec(ref FinalNodes, 0, Target);
    }

    private List<EntityID> GetAllAtRec(ref NativeArray<RadixTree.Node> Nodes, int Index, Rect Target)
    {
        List<EntityID> List = new();
        var Node = Nodes[Index];
        Vector3 Size = Node.Max - Node.Min;
        Rect Temp = new(new Vector2(Node.Min.x, Node.Min.z), new(Size.x, Size.z));
        if (!Temp.Overlaps(Target))
            return List;

        Node.GetLeft(out var LeftIndex, out var bIsLeftLeaf);
        Node.GetRight(out var RightIndex, out var bIsRightLeaf);
        if (bIsLeftLeaf)
        {
            var LeftTemp = Components[FinalIDIndices[LeftIndex]].GetPositionXZ();
            if (Target.Contains(LeftTemp))
            {
                List.Add(OriginalIDs[FinalIDIndices[LeftIndex]]);
            }
        }
        else
        {
            List.AddRange(GetAllAtRec(ref Nodes, LeftIndex, Target));
        }
        if (bIsRightLeaf)
        {
            var RightTemp = Components[FinalIDIndices[RightIndex]].GetPositionXZ();
            if (Target.Contains(RightTemp))
            {
                List.Add(OriginalIDs[FinalIDIndices[RightIndex]]);
            }
        }
        else
        {
            List.AddRange(GetAllAtRec(ref Nodes, RightIndex, Target));
        }
        return List;
    }

    private const int LSBWidth = 2;
    private const int MortonWidth = 32;
    private const int BucketSize = 4;
    private readonly static int ThreadCount = 1;// JobsUtility.JobWorkerCount;
}
