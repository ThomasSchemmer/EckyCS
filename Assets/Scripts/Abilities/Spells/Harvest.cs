using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Harvest", menuName = "ScriptableObjects/Abilities/Util/Harvest", order = 0)]
public class Harvest : GameplayAbility
{
    private LocationSystem Locations;
    private EckyCS ECS;
    private Dictionary<ComponentGroupIdentifier, List<EntityID>> FoundPlantIDs;

    private GameObject Preview;
    private float Scale = 1;
    private float Offset = 3;

    private bool ArePlantsInRange()
    {
        if (Locations == null || ECS == null)
            return false;

        FoundPlantIDs = new();
        Vector3 Target = GetTargetPosition();
        Preview.SetActive(true);
        Preview.transform.position = Target;
        Preview.transform.localScale = Vector3.one * Scale;
        return Locations.TryGetEntityListsAt<GrowthComponent, TransformComponent>(Target, out FoundPlantIDs, Scale);
    }

    public Vector3 GetTargetPosition()
    {
        Vector3 Target = AssignedToBehaviour.transform.position + AssignedToBehaviour.transform.forward * Offset;
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
        List<EntityID> ItemIDs = new();
        Dictionary<EntityID, (ItemType, Vector3)> ItemInfos = new();
        // check which plants are harvested and generate drops
        foreach (var Pair in FoundPlantIDs)
        {

            var PlantSet = ECS.GetSet(Pair.Key);
            int GrowthIndex = Pair.Key.GetSelfIndexOf(typeof(GrowthComponent));
            int TransformIndex = Pair.Key.GetSelfIndexOf(typeof(TransformComponent));
            if (GrowthIndex < 0 || TransformIndex < 0)
                continue;

            PlantSet.ForEachEntityFrom(Pair.Value, (Group, Ptrs, Index) =>
            {
                var GrowthPtr = ((GrowthComponent*)Ptrs[GrowthIndex]) + Index;
                var TransformPtr = ((TransformComponent*)Ptrs[TransformIndex]) + Index;
                if (GrowthPtr->Growth != 2)
                    return false;

                GrowthComponent.PlantType Type = GrowthPtr->Plant;
                if (!EntityGenerator.TryCreate(out Item Item))
                    return false;

                ItemIDs.Add(Item.ID);
                ItemInfos.Add(Item.ID, new(
                    PlantItemMap.GetFor(Type),
                    TransformPtr->GetPosition()
                ));
                return true;
            });
            PlantSet.RemoveRange(Pair.Value);
        }

        // move drops to player location
        foreach (var GroupID in ECS.GetProvider().Get<ItemComponent, TransformComponent>().Groups)
        {
            var Set = ECS.GetSet(GroupID);
            int TransformIndex = GroupID.GetSelfIndexOf(typeof(TransformComponent));
            int ItemIndex = GroupID.GetSelfIndexOf(typeof(ItemComponent));
            if (TransformIndex < 0 || ItemIndex < 0)
                continue;

            Set.ForEachEntityFrom(ItemIDs, (Group, Ptrs, Index) =>
            {
                var TransformPtr = ((TransformComponent*)Ptrs[TransformIndex]) + Index;
                var ItemPtr = ((ItemComponent*)Ptrs[ItemIndex]) + Index;
                var IDPtr = ((EntityID*)Ptrs[Ptrs.Length - 1]) + Index;
                var Info = ItemInfos[*IDPtr];
                Vector3 RandomPos = Info.Item2 + new Vector3(Random.Range(-1.0f, 1.0f), 0, Random.Range(-1.0f, 1.0f));
                TransformPtr->PosX = RandomPos.x;
                TransformPtr->PosY = RandomPos.y;
                TransformPtr->PosZ = RandomPos.z;
                ItemPtr->Type = Info.Item1;
                ItemPtr->Amount = 1;
                return true;
            });
        }
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
        return FoundPlantIDs;
    }

    private void OnDestroy()
    {
        _OnEndAbility -= Hide;
    }
}
