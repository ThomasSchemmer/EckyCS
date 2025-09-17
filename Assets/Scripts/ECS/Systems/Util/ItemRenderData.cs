using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;
using UnityEngine.Rendering;

/** 
 * Render data specific for items, adds additional information in the form
 * of the type buffer
 */
public class ItemRenderData : RenderData
{
    private ComputeBuffer ItemBuffer;

    public override void Create(int Count, int DataStride)
    {
        base.Create(Count, DataStride);

        ItemBuffer = new ComputeBuffer(
            Count,
            ComponentAllocator.GetSize(typeof(ItemComponent)),
            ComputeBufferType.Structured
        );
    }

    public override unsafe void UpdateBuffers(ComponentGroupIdentifier GroupID, void*[] Ptrs, void* Data, int Count)
    {
        base.UpdateBuffers(GroupID, Ptrs, Data, Count);

        // for logic/info see base
        int ItemTarget = GroupID.GetSelfIndexOf(typeof(ItemComponent));
        NativeArray<ItemComponent> Items = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<ItemComponent>(Ptrs[ItemTarget], Count, Allocator.None);
        AtomicSafetyHandle ItemSH = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Items, ItemSH);

        ItemBuffer.SetData(Items);

        AtomicSafetyHandle.CheckDeallocateAndThrow(ItemSH);
        AtomicSafetyHandle.Release(ItemSH);
    }

    protected override void SetRenderInfo(RenderInfo Info, ref CommandBuffer Cmd, ComputeShader CullingCompute)
    {
        base.SetRenderInfo(Info, ref Cmd, CullingCompute);
        if (Info is not ItemInfo CombinedInfo)
            return;

        Info.Mat.SetBuffer("ItemBuffer", ItemBuffer);
        Info.Mat.SetTexture("_MainTex", CombinedInfo.CombinedTex);
        Info.Mat.SetVector("_Size", CombinedInfo.Size);
    }

    public override void Dispose()
    {
        base.Dispose();
        ItemBuffer?.Dispose();
        ItemBuffer = null;
    }

    public override bool ShouldCheckData()
    {
        return false;
    }
}
