using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GrowthSystem : EckyCSSystem
{
    private EckyCS ECS;
    private Dictionary<GrowthComponent.PlantType, List<(int, int)>> GrowthMap = new();

    public void Tick(float Delta)
    {
        unsafe
        {
            ECS.GetProvider().Get<GrowthComponent>().ForEachGroup((Group, Ptrs, Count) =>
            {
                Grow(Group, Ptrs, Count);
            });
        }
    }

    private unsafe void Grow(ComponentGroupIdentifier Group, void*[] Ptrs, int Count)
    {

        int GrowthTarget = Group.GetSelfIndexOf(typeof(GrowthComponent));
        if (GrowthTarget < 0)
            return;

        for (int i = 0; i < Count; i++)
        {
            var Ptr = (GrowthComponent*)Ptrs[GrowthTarget] + i;
            if (!GrowthMap.TryGetValue(Ptr->Plant, out var List))
                continue;

            var TimeDiff = Time.realtimeSinceStartup - Ptr->PlantedAtS;
            foreach (var Entry in List)
            {
                if (TimeDiff < Entry.Item1)
                    continue;

                Ptr->Growth = Entry.Item2;
            }
        }
    }

    public void Destroy()
    {
    }

    private void Init()
    {
        // try to find any registered plant maps
        if (!ECS.TryGetSystems<RenderSystem>(out var List))
            return;


        foreach (var System in List)
        {
            if (System is not PlantRenderSystem PlantSystem)
                continue;

            foreach (var Info in PlantSystem.Infos)
            {
                if (Info is not PlantInfo Plant)
                    continue;

                if (!GrowthMap.ContainsKey(Plant.PlantType))
                {
                    GrowthMap.Add(Plant.PlantType, new());
                }
                GrowthMap[Plant.PlantType].Add(new(Plant.GrowthTimeS, (int)Info.TargetData));
            }
        }
    }

    public void StartSystem()
    {
        Game.RunAfterServiceInit((EckyCS ECS) =>
        {
            this.ECS = ECS;
            Init();
        });
    }
}
