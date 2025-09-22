using System.Collections;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using UnityEngine;
using static UnityEngine.GraphicsBuffer;

public class ItemPickupSystem : MonoBehaviour, EckyCSSystem
{
    private EckyCS ECS;
    private List<GameplayAbilityBehaviour> Players = new();
    private LocationSystem Locations;
    private InventoryService Inventory;

    private readonly float MagnetRange = 15;
    private readonly float MagnetStrength = 2;
    private readonly float PickupRange = 0.75f;

    public unsafe void Tick(float Delta)
    {
        if (Locations == null || Players == null)
            return;

        foreach (var Player in Players)
        {
            Vector3 Target = Player.transform.position;
            Target.y = 0;
            if (!Locations.TryGetEntityListsAt<TransformComponent, ItemComponent>(Target, out var FoundGroups, MagnetRange))
                return;

            foreach (var Tuple in FoundGroups)
            {
                HandleGroup(Tuple, Target, Delta);
            }
        }
    }

    private unsafe void HandleGroup(KeyValuePair<ComponentGroupIdentifier, List<EntityID>> Tuple, Vector3 Target, float Delta)
    {
        var Set = ECS.GetSet(Tuple.Key);
        int TransformIndex = Tuple.Key.GetSelfIndexOf(typeof(TransformComponent));
        int ItemIndex = Tuple.Key.GetSelfIndexOf(typeof(ItemComponent));
        if (TransformIndex < 0 || ItemIndex < 0)
            return;

        List<EntityID> ItemsToRemove = new();
        Set.ForEachEntityFrom(Tuple.Value, (Group, Ptrs, Index) =>
        {
            var TransformPtr = ((TransformComponent*)Ptrs[TransformIndex]) + Index;
            var ItemPtr = ((ItemComponent*)Ptrs[ItemIndex]) + Index;
            var IDPtr = ((EntityID*)Ptrs[Ptrs.Length - 1]) + Index;
            Vector3 Pos = TransformPtr->GetPosition();
            Vector3 Dir = (Target - Pos).normalized;
            Pos += Delta * MagnetStrength * Dir;
            TransformPtr->SetPosition(Pos);

            float Distance = Vector3.Distance(Pos, Target);
            if (Distance < PickupRange)
            {
                Inventory.AddItem(ItemPtr->Type, 1);
                ItemsToRemove.Add(*IDPtr);
            }
            return true;
        });

        Set.RemoveRange(ItemsToRemove);
    }

    public void Start()
    {
        Game.RunAfterServicesInit((GameplayAbilitySystem GAS, EckyCS ECS) =>
        {
            GAS.RunAfterBehaviourRegistered(GameplayAbilitySystem.Type.Player, (Behaviour) =>
            {
                Players.Add(Behaviour);
            },
            false
            );
            this.ECS = ECS;
            ECS.TryGetSystem(out Locations);
            ECS.AddSystem(this);
        });
        Game.RunAfterServiceInit((InventoryService Inventory) =>
        {
            this.Inventory = Inventory;
        });
    }
    public void StartSystem() { }
    public void Destroy()
    {
    }
}
