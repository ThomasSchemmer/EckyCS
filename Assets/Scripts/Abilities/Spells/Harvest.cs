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
    private float Scale = 1;

    private bool ArePlantsInRange()
    {
        if (Locations == null || ECS == null)
            return false;

        FoundIDs = new();
        Vector3 Target = GetTargetPosition();
        Preview.SetActive(true);
        Preview.transform.position = Target;
        Preview.transform.localScale = Vector3.one * Scale;
        return Locations.TryGetEntityListsAt<GrowthComponent, TransformComponent>(Target, out FoundIDs, Scale);
    }

    public Vector3 GetTargetPosition()
    {
        Vector3 Target = AssignedToBehaviour.transform.position + AssignedToBehaviour.transform.forward;
        Target.y = 0.5f;
        return Target; 
    }


    protected override void TickInternal(float Delta)
    {
        base.TickInternal(Delta);

        if (Status != State.Committed)
            return;
        
        // query it before the mouse down to update the preview cue implicitly
        if (!ArePlantsInRange())
            return;

        if (!Input.GetMouseButtonDown(0))
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
                // todo: inefficient self index search for every entity!
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
        Hide();
        _OnEndAbility += Hide;
    }

    public override void OnGranted()
    {
        base.OnGranted();
        Game.RunAfterServiceInit((EckyCS ECS) =>
        {
            Init(ECS);
        });
    }

    private void Hide()
    {
        if (Preview) Preview.SetActive(false);
    }

    public Dictionary<ComponentGroupIdentifier, List<EntityID>> GetTargets()
    {
        return FoundIDs;
    }

    private void OnDestroy()
    {
        _OnEndAbility -= Hide;
    }
}
