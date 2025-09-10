using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using Unity.Collections;
using UnityEngine;
using UnityEngine.Profiling;
using UnityEngine.Rendering;

/** 
 * Renders every entity with both SpriteComponent and TransformComponent,
 * by iterating and batching each group. This is obviously slower for a small amount of entities, 
 * so be careful!
 * The updating occurs async from rendering as well, so there might be some inconsistencies
 */
public abstract class RenderSystem : MonoBehaviour, EckyCSSystem
{
    public void StartSystem() {
        // handled in subclasses 
    }
    public abstract void Destroy();
    public abstract void AddToRenderBuffer(ref CommandBuffer Cmd);
}


public abstract class RenderSystem<D, T> : RenderSystem where T : RenderInfo where D : RenderData, new()
{
    public Material BaseMat;
    public ComputeShader CullingCompute;

    protected EckyCS ECS;

    public Dictionary<ComponentGroupIdentifier, D> Datas = new();

    public List<T> Infos = new();

    public virtual void Start()
    {
        Game.RunAfterServiceInit((EckyCS ECS) =>
        {
            this.ECS = ECS;
            ECS.AddSystem(this);
            Init();
        });
    }

    public unsafe void Register(ComponentGroupIdentifier Group, void*[] Ptrs, void* Data, int DataStride, int Count)
    {
        if (Datas.ContainsKey(Group) && Count > Datas[Group].Count)
        {
            Datas[Group].Dispose();
            Datas.Remove(Group);
        }
        if (!Datas.ContainsKey(Group))
        {
            Datas.Add(Group, new());
            Datas[Group].Create(Count, DataStride);
        }

        Datas[Group].UpdateBuffers(Group, Ptrs, Data, Count);
    }

    public void LateTick(float Delta)
    {
    }


    public void Init()
    {
        foreach (var Info in Infos)
        {
            Info.Init(BaseMat);
        }
    }

    public override void AddToRenderBuffer(ref CommandBuffer Cmd)
    {
        if (Cmd == null)
            return;

        foreach (var DataPair in Datas)
        {
            DataPair.Value.AddToRenderBuffer(ref Infos, ref Cmd, CullingCompute);
        }
    }

    public override void Destroy()
    {
        foreach (var Tuple in Datas)
        {
            Datas[Tuple.Key].Dispose();
        }
        foreach (var Infos in Infos)
        {
            Infos.Dispose();
        }
        Datas.Clear();
        Infos.Clear();
    }

}
