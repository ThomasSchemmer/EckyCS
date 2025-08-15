using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static UnityEngine.GraphicsBuffer;

/**
 * Provides requirements for splitting an EntityGroup into different drawcalls
 * regarding target data (aka "is this a fitting mesh rn?")
 */
[Serializable]
public class RenderInfo
{
    public Mesh Mesh;
    public uint TargetData;
    public Vector3 Scale;

    [HideInInspector] public ComponentGroupIdentifier GroupID;
    [HideInInspector] public int Count;

    // the batcher apparently likes instanced mats, not MPBs
    [HideInInspector] public Material Mat;
    [HideInInspector] public ComputeBuffer ArgsBuffer;
    [HideInInspector] public ComputeBuffer PositionAppendBuffer;

    private IndirectDrawIndexedArgs[] CommandData;

    public void Dispose()
    {
        PositionAppendBuffer?.Dispose();
        PositionAppendBuffer = null;
        ArgsBuffer?.Dispose();
        ArgsBuffer = null;
    }

    public void Init(Material BaseMat)
    {
        CommandData = new IndirectDrawIndexedArgs[CommandCount];
        ArgsBuffer = new ComputeBuffer(CommandCount, IndirectDrawIndexedArgs.size, ComputeBufferType.IndirectArguments);
        PositionAppendBuffer = new ComputeBuffer(
            AppendCount,
            ComponentAllocator.GetSize(typeof(TransformComponent)), 
            ComputeBufferType.Append
        );
        Mat = Material.Instantiate(BaseMat);

        // rest is either 0 or will be filled by compute
        CommandData[0].indexCountPerInstance = Mesh.GetIndexCount(0);
        ArgsBuffer.SetData(CommandData);
    }


    public const int CommandCount = 1;
    public const int AppendCount = 1000;
}
