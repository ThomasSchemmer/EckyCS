using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "PlantItem", menuName = "ScriptableObjects/Maps/PlantItem", order = 0)]
public class PlantItemMap : ScriptableObject
{
    public SerializedDictionary<GrowthComponent.PlantType, ItemComponent.ItemType> Mapping = new();

    // todo: should be refactored into resource lookup service
    public static PlantItemMap Instance;

    public void OnEnable()
    {
        Instance = this;
    }

    public static ItemComponent.ItemType GetFor(GrowthComponent.PlantType Plant)
    {
        if (Instance == null)
            return ItemComponent.ItemType.DEFAULT;

        if (!Instance.Mapping.ContainsKey(Plant))
            return ItemComponent.ItemType.DEFAULT;

        return Instance.Mapping[Plant];
    }
}
