using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;
using UnityEngine.Rendering;

/** 
 * Render data specific for plants, adds additional information in the form
 * of the highlight buffer
 */
public class PlantRenderData : RenderData
{

    private ComputeBuffer HighlightBuffer;

    public override void Create(int Count, int DataStride)
    {
        base.Create(Count, DataStride);

        HighlightBuffer = new ComputeBuffer(
            Count,
            ComponentAllocator.GetSize(typeof(HighlightComponent)),
            ComputeBufferType.Structured
        );
    }

    public override unsafe void UpdateBuffers(ComponentGroupIdentifier GroupID, void*[] Ptrs, void* Data, int Count)
    {
        base.UpdateBuffers(GroupID, Ptrs, Data, Count);

        // for logic/info see base
        int HighlightTarget = GroupID.GetSelfIndexOf(typeof(HighlightComponent));
        NativeArray<HighlightComponent> Highlights = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<HighlightComponent>(Ptrs[HighlightTarget], Count, Allocator.None);
        AtomicSafetyHandle HighlightsSH = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Highlights, HighlightsSH);

        HighlightBuffer.SetData(Highlights);

        AtomicSafetyHandle.CheckDeallocateAndThrow(HighlightsSH);
        AtomicSafetyHandle.Release(HighlightsSH);
    }

    protected override void SetRenderInfo(RenderInfo Info, ref CommandBuffer Cmd, ComputeShader CullingCompute)
    {
        base.SetRenderInfo(Info, ref Cmd, CullingCompute);
        Info.Mat.SetBuffer("HighlightBuffer", HighlightBuffer);
    }

    public override void Dispose()
    {
        base.Dispose();
        HighlightBuffer?.Dispose();
        HighlightBuffer = null;
    }
}
