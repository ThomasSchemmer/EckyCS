using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Harvest", menuName = "ScriptableObjects/Abilities/Util/Harvest", order = 0)]
public class Harvest : GameplayAbility
{
    private LocationSystem Locations;
    private EckyCS ECS;
    private Dictionary<ComponentGroupIdentifier, List<EntityID>> FoundIDs;

    private GameObject Preview;

    private bool ArePlantsInRange()
    {
        if (Locations == null || ECS == null)
            return false;

        FoundIDs = new();
        Vector3 Target = AssignedToBehaviour.transform.position + AssignedToBehaviour.transform.forward;
        Target.y = 0.5f;
        Preview.transform.position = Target;
        return Locations.TryGetEntityListsAt<GrowthComponent, TransformComponent>(Target, out FoundIDs, 1f);
    }


    public override void Tick(float Delta)
    {
        base.Tick(Delta);

        if (Status != State.Committed)
            return;

        if (!Input.GetMouseButtonDown(0))
            return;

        if (!ArePlantsInRange())
            return;

        Execute();
    }

    private unsafe void Execute()
    {
        Dictionary<GrowthComponent.PlantType, int> Harvested = new();
        foreach (var Pair in FoundIDs)
        {

            var Set = ECS.GetSet(Pair.Key);
            Set.ForEachEntityFrom(Pair.Value, (Group, Ptrs, Index) =>
            {
                int GrowthIndex = Group.GetSelfIndexOf(typeof(GrowthComponent));
                if (GrowthIndex < 0)
                    return false;

                var Ptr = ((GrowthComponent*)Ptrs[GrowthIndex]) + Index;
                if (Ptr->Growth != 2)
                    return false;

                GrowthComponent.PlantType Type = Ptr->Plant;
                if (!Harvested.ContainsKey(Type)){
                    Harvested.Add(Type, 0);
                }
                Harvested[Type]++;
                return true;
            });
            Set.RemoveRange(Pair.Value);
        }

        int Count = 0;
        foreach (var item in Harvested)
        {
            Count += item.Value;
        }
        Debug.Log("Harvested: " + Count);
    }

    private void Init(EckyCS ECS)
    {
        if (!ECS.TryGetSystem(out Locations))
            return;

        this.ECS = ECS;
        Preview = GameObject.CreatePrimitive(PrimitiveType.Cube);
    }

    public override void OnGranted()
    {
        base.OnGranted();
        Game.RunAfterServiceInit((EckyCS ECS) =>
        {
            Init(ECS);
        });
    }
}
