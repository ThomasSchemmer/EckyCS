using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlantRenderSystem : RenderSystem
{
    public unsafe void Update()
    {
        ECS.GetProvider().Get<TransformComponent, RenderComponent, GrowthComponent>().ForEachGroup((Group, Ptrs, Count) =>
        {
            int DataTarget = Group.GetSelfIndexOf(typeof(GrowthComponent));
            int DataStride = ComponentAllocator.GetSize(typeof(GrowthComponent));
            Register(Group, Ptrs, Ptrs[DataTarget], DataStride, Count);
        });
    }
}
