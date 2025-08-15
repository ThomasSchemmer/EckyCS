using Codice.Client.BaseCommands;
using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Mathematics;
using UnityEngine;
using UnityEngine.Rendering;
using static UnityEngine.GraphicsBuffer;


/** Holds necessary data for rendering a single entity group (ie a chunk of memory)
 * This will then be split into different DrawCalls, one for each provided Info
 * Any compute culling call needs specific data to test if it should be rendered
 * Store that data in a RawBuffer and provide struct length info 
 * 
 * TODO: async nature of updating/rendering might invalidate pointers!
 */
[Serializable]
public unsafe class RenderData
{
    // only one of each buffer, as they are shared
    private ComputeBuffer PositionBuffer;
    private ComputeBuffer CullingDataBuffer;

    private const int CullKernel = 0;
    private int DataStride;
    public int Count;

    private Camera Cam;

    public void Create(int Count, int DataStride)
    {

        PositionBuffer = new ComputeBuffer(
            Count, 
            ComponentAllocator.GetSize(typeof(TransformComponent)), 
            ComputeBufferType.Structured
        );
        CullingDataBuffer = new ComputeBuffer(Count, DataStride, ComputeBufferType.Raw);

        this.DataStride = DataStride;
        this.Cam = Camera.main;
        this.Count = Count;
    }


    public void UpdateBuffers(ComponentGroupIdentifier GroupID, void*[] Ptrs, void* Data, int Count)
    {
        // would be good to move this into its own functions, but calling generic functions with 
        // ref params and no way to infer their type is.. mid at best. So we dupe the code
        int TransformTarget = GroupID.GetSelfIndexOf(typeof(TransformComponent));
        NativeArray<TransformComponent> Transforms = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>(Ptrs[TransformTarget], Count, Allocator.None);
        NativeArray<byte> DataArray = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<byte>(Data, Count * DataStride, Allocator.None);

        AtomicSafetyHandle TransformSH = AtomicSafetyHandle.Create();
        AtomicSafetyHandle DataSH = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Transforms, TransformSH);
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref DataArray, DataSH);

        PositionBuffer.SetData(Transforms);
        CullingDataBuffer.SetData(DataArray);

        // we don't need to deallocate the NativeArrays as they are repurposed pointers anyway
        AtomicSafetyHandle.CheckDeallocateAndThrow(TransformSH);
        AtomicSafetyHandle.CheckDeallocateAndThrow(DataSH);
        AtomicSafetyHandle.Release(TransformSH);
        AtomicSafetyHandle.Release(DataSH);
    }

    public void AddToRenderBuffer(ref List<RenderInfo> Infos, ref CommandBuffer Cmd, ComputeShader CullingCompute)
    {
        Vector3 CamForward = Vector3.Normalize(Vector3.Cross(new(0, 1, 0), Cam.transform.forward));

        foreach (var Info in Infos)
        {
            ComputeCull(Info, CullingCompute);

            Info.Mat.SetBuffer("PositionBuffer", Info.PositionAppendBuffer);
            Info.Mat.SetVector("_CamForward", CamForward);
            Info.Mat.SetVector("_CamUp", Cam.transform.up);
            Info.Mat.SetVector("_CamRight", Cam.transform.right);
            Info.Mat.SetVector("_CamPos", Cam.transform.position);
            Info.Mat.SetVector("_Scale", Info.Scale);

            Cmd.DrawMeshInstancedIndirect(
                Info.Mesh,
                0,
                Info.Mat,
                0,
                Info.ArgsBuffer
            );
        }
    }

    private void ComputeCull(RenderInfo Info, ComputeShader CullingCompute)
    {
        CullingCompute.SetBuffer(CullKernel, "PositionAppendBuffer", Info.PositionAppendBuffer);
        CullingCompute.SetBuffer(CullKernel, "PositionBuffer", PositionBuffer);
        CullingCompute.SetBuffer(CullKernel, "CullingDataBuffer", CullingDataBuffer);
        CullingCompute.SetInt("DataStride", DataStride);
        CullingCompute.SetInt("GroupCountX", 8);
        CullingCompute.SetInt("GroupCountY", 8);
        CullingCompute.SetInt("GroupCountZ", 1);
        CullingCompute.SetInt("TotalCount", Count);
        CullingCompute.SetInt("TargetPlant", (int)Info.TargetData);

        // cull anything of the wrong type //TODO: make type/data unspecific
        Info.PositionAppendBuffer.SetCounterValue(0);
        CullingCompute.Dispatch(CullKernel, 8, 8, 1);

        // tell the gpu how many entities there are
        // note: the offset lets us write into [1], which is the instanceCount - and not the vertices per instance!
        ComputeBuffer.CopyCount(
            Info.PositionAppendBuffer, 
            Info.ArgsBuffer, 
            sizeof(uint)
        );
    }

    public void Dispose()
    {
        PositionBuffer?.Dispose();
        PositionBuffer = null;
        CullingDataBuffer?.Dispose();
        CullingDataBuffer = null;
    }
}