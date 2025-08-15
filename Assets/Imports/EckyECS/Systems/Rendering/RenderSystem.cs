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
 */
public abstract class RenderSystem : MonoBehaviour, ECSSystem
{
    public Material BaseMat;
    public ComputeShader CullingCompute;

    public List<RenderInfo> Infos = new();

    protected ECS ECS;

    public Dictionary<ComponentGroupIdentifier, RenderData> Datas = new();

    public void StartSystem() {

    }

    public void Start()
    {
        Game.RunAfterServiceInit((ECS ECS) =>
        {
            this.ECS = ECS;
            ECS.AddSystem(this);
            Init();
        });
    }

    private void Init()
    {
        foreach (var Info in Infos)
        {
            Info.Init(BaseMat);
        }
    }

    public void AddToRenderBuffer(ref CommandBuffer Cmd)
    {
        if (Cmd == null)
            return;

        foreach (var DataPair in Datas)
        {
            DataPair.Value.AddToRenderBuffer(ref Infos, ref Cmd, CullingCompute);
        }
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

    public void Destroy()
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

    public void LateTick(float Delta)
    {
    }
}
