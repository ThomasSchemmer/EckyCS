using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "PlantItem", menuName = "ScriptableObjects/Maps/PlantItem", order = 0)]
public class PlantItemMap : ScriptableObject
{
    public SerializedDictionary<GrowthComponent.PlantType, ItemType> Mapping = new();

    // todo: should be refactored into resource lookup service
    public static PlantItemMap Instance;

    public void OnEnable()
    {
        Instance = this;
    }

    public static ItemType GetFor(GrowthComponent.PlantType Plant)
    {
        if (Instance == null)
        {
            Instance = Resources.Load("Util/PlantItem") as PlantItemMap;
        }
        if (Instance == null)
            return ItemType.DEFAULT;

        if (!Instance.Mapping.ContainsKey(Plant))
            return ItemType.DEFAULT;

        return Instance.Mapping[Plant];
    }
}
