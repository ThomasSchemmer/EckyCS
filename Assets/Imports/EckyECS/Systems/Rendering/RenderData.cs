using Codice.Client.BaseCommands;
using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Mathematics;
using UnityEngine;
using UnityEngine.Rendering;


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
    private ComputeBuffer IDBuffer;

    private const int CullKernel = 0;
    private int DataStride;
    public int Count;

    private Camera Cam;

    public virtual void Create(int Count, int DataStride)
    {
        PositionBuffer = new ComputeBuffer(
            Count, 
            ComponentAllocator.GetSize(typeof(TransformComponent)), 
            ComputeBufferType.Structured
        );
        IDBuffer = new ComputeBuffer(
            Count, 
            sizeof(uint), 
            ComputeBufferType.Structured
        );
        CullingDataBuffer = new ComputeBuffer(Count, DataStride, ComputeBufferType.Raw);

        this.DataStride = DataStride;
        this.Cam = Camera.main;
        this.Count = Count;
    }


    public virtual void UpdateBuffers(ComponentGroupIdentifier GroupID, void*[] Ptrs, void* Data, int Count)
    {
        // would be good to move this into its own functions, but calling generic functions with 
        // ref params and no way to infer their type is.. mid at best. So we dupe the code
        int TransformTarget = GroupID.GetSelfIndexOf(typeof(TransformComponent));
        NativeArray<TransformComponent> Transforms = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<TransformComponent>(Ptrs[TransformTarget], Count, Allocator.None);
        NativeArray<byte> DataArray = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<byte>(Data, Count * DataStride, Allocator.None);
        NativeArray<uint> IDArray = NativeArrayUnsafeUtility.ConvertExistingDataToNativeArray<uint>(Ptrs[Ptrs.Length - 1], Count, Allocator.None);

        AtomicSafetyHandle TransformSH = AtomicSafetyHandle.Create();
        AtomicSafetyHandle DataSH = AtomicSafetyHandle.Create();
        AtomicSafetyHandle IDSH = AtomicSafetyHandle.Create();
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref Transforms, TransformSH);
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref DataArray, DataSH);
        NativeArrayUnsafeUtility.SetAtomicSafetyHandle(ref IDArray, IDSH);

        PositionBuffer.SetData(Transforms);
        CullingDataBuffer.SetData(DataArray);
        IDBuffer.SetData(IDArray);

        // we don't need to deallocate the NativeArrays as they are repurposed pointers anyway
        AtomicSafetyHandle.CheckDeallocateAndThrow(TransformSH);
        AtomicSafetyHandle.CheckDeallocateAndThrow(DataSH);
        AtomicSafetyHandle.CheckDeallocateAndThrow(IDSH);
        AtomicSafetyHandle.Release(TransformSH);
        AtomicSafetyHandle.Release(DataSH);
        AtomicSafetyHandle.Release(IDSH);
    }

    public void AddToRenderBuffer<T>(ref List<T> Infos, ref CommandBuffer Cmd, ComputeShader CullingCompute) where T : RenderInfo
    {
        Vector3 CamForward = Vector3.Normalize(Vector3.Cross(new(0, 1, 0), Cam.transform.forward));

        foreach (var Info in Infos)
        {
            SetRenderInfo(Info, ref Cmd, CullingCompute);

            Info.Mat.SetVector("_CamForward", CamForward);
            Cmd.DrawMeshInstancedIndirect(
                Info.GetMesh(),
                0,
                Info.Mat,
                Info.ShaderPass,
                Info.ArgsBuffer
            );
        }
    }

    protected virtual void SetRenderInfo(RenderInfo Info, ref CommandBuffer Cmd, ComputeShader CullingCompute)
    {
        ComputeCull(Info, ref Cmd, CullingCompute);
        Info.Mat.SetBuffer("PositionBuffer", Info.PositionAppendBuffer);
        Info.Mat.SetVector("_CamUp", Cam.transform.up);
        Info.Mat.SetVector("_CamRight", Cam.transform.right);
        Info.Mat.SetVector("_CamPos", Cam.transform.position);
        Info.Mat.SetVector("_Scale", Info.Scale);
        // type 0 is reserved for general highlights in UV lookup table
        Info.Mat.SetInt("_Type", (int)Info.TargetData + 1);
    }

    private void ComputeCull(RenderInfo Info, ref CommandBuffer Cmd, ComputeShader CullingCompute)
    {
        Cmd.SetComputeBufferParam(CullingCompute, CullKernel, "PositionAppendBuffer", Info.PositionAppendBuffer);
        Cmd.SetComputeBufferParam(CullingCompute, CullKernel, "PositionBuffer", PositionBuffer);
        Cmd.SetComputeBufferParam(CullingCompute, CullKernel, "CullingDataBuffer", CullingDataBuffer);
        Cmd.SetComputeBufferParam(CullingCompute, CullKernel, "IDBuffer", IDBuffer);
        Cmd.SetComputeIntParam(CullingCompute, "DataStride", DataStride);
        Cmd.SetComputeIntParam(CullingCompute, "GroupCountX", 8);
        Cmd.SetComputeIntParam(CullingCompute, "GroupCountY", 8);
        Cmd.SetComputeIntParam(CullingCompute, "GroupCountZ", 1);
        Cmd.SetComputeIntParam(CullingCompute, "TotalCount", Count);
        Cmd.SetComputeIntParam(CullingCompute, "TargetData", (int)Info.TargetData);
        Cmd.SetComputeIntParam(CullingCompute, "CheckData", ShouldCheckData() ? 1 : 0);

        Cmd.SetBufferCounterValue(Info.PositionAppendBuffer, 0);
        Cmd.DispatchCompute(CullingCompute, CullKernel, 8, 8, 1);

        // tell the gpu how many entities there are
        // note: the offset lets us write into [1], which is the instanceCount - and not the vertices per instance!
        Cmd.CopyCounterValue(
            Info.PositionAppendBuffer, 
            Info.ArgsBuffer, 
            sizeof(uint)
        );
    }

    public virtual void Dispose()
    {
        PositionBuffer?.Dispose();
        PositionBuffer = null;
        CullingDataBuffer?.Dispose();
        CullingDataBuffer = null;
        IDBuffer?.Dispose();
        IDBuffer = null;
    }

    public virtual bool ShouldCheckData()
    {
        return true;
    }
}