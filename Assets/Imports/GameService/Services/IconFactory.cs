using System;
using System.Security.AccessControl;
using TMPro;
using Unity.VectorGraphics;
using UnityEngine;
using UnityEngine.UI;

public class IconFactory : GameService
{
    public SerializedDictionary<MiscellaneousType, Sprite> AvailableMiscellaneous = new();
    public SerializedDictionary<AbilityType, Sprite> AvailableAbilities = new();
    public SerializedDictionary<ItemType, Sprite> AvailableItems = new();

    private Sprite PlaceholderSprite;

    private GameObject NumberedIconPrefab;
    private GameObject AbilitySlotPrefab;
    private GameObject BooleanSettingPrefab;
    private GameObject IntSettingPrefab;

    public enum MiscellaneousType
    {
        Temp
    }

    public void Refresh()
    {
        LoadMiscellaneous();
        LoadAbilities();
        LoadItems();
        LoadPlaceholder();
        LoadPrefabs();
    }

    private void LoadPrefabs()
    {
        NumberedIconPrefab = Resources.Load("UI/NumberedIcon") as GameObject;
        AbilitySlotPrefab = Resources.Load("UI/Slot") as GameObject;
    }

    private void LoadPlaceholder()
    {
        GameObject MeshObject = Resources.Load("Icons/UI/Placeholder") as GameObject;
        if (!MeshObject || !MeshObject.GetComponent<SpriteRenderer>())
            return;

        Sprite Sprite = MeshObject.GetComponent<SpriteRenderer>().sprite;
        if (!Sprite)
            return;

        PlaceholderSprite = Sprite;
    }

    private void LoadMiscellaneous()
    {
        AvailableMiscellaneous.Clear();
        var MiscTypes = Enum.GetValues(typeof(MiscellaneousType));
        foreach (var MiscType in MiscTypes)
        {
            GameObject MeshObject = Resources.Load("Icons/Misc/" + MiscType) as GameObject;
            if (!MeshObject || !MeshObject.GetComponent<SpriteRenderer>())
                continue;

            Sprite Sprite = MeshObject.GetComponent<SpriteRenderer>().sprite;
            if (!Sprite)
                continue;

            AvailableMiscellaneous.Add((MiscellaneousType)MiscType, Sprite);
        }
    }

    private void LoadAbilities()
    {
        AvailableAbilities.Clear();
        var AbilityTypes = Enum.GetValues(typeof(AbilityType));
        foreach (var AbilityType in AbilityTypes)
        {
            GameObject GO = Resources.Load("Icons/Abilities/" + AbilityType) as GameObject;
            if (!GO || !GO.GetComponent<SpriteRenderer>())
                continue;

            Sprite Sprite = GO.GetComponent<SpriteRenderer>().sprite;
            if (!Sprite)
                continue;

            AvailableAbilities.Add((AbilityType)AbilityType, Sprite);
        }
    }

    private void LoadItems()
    {
        AvailableItems.Clear();
        var ItemTypes = Enum.GetValues(typeof(ItemType));
        foreach (var ItemType in ItemTypes)
        {
            var GO = Resources.Load("Icons/Items/" + ItemType) as GameObject;
            if (!GO.TryGetComponent<SpriteRenderer>(out var Rnd))
                continue;

            AvailableItems.Add((ItemType)ItemType, Rnd.sprite);
        }
    }


    public Sprite GetIconForMisc(MiscellaneousType Type)
    {
        if (!AvailableMiscellaneous.ContainsKey(Type))
            return PlaceholderSprite;

        return AvailableMiscellaneous[Type];
    }

    public Sprite GetIconForAbility(AbilityType Type)
    {
        if (!AvailableAbilities.ContainsKey(Type))
            return PlaceholderSprite;

        return AvailableAbilities[Type];
    }

    public Sprite GetIconForItem(ItemType Type)
    {
        if (!AvailableItems.ContainsKey(Type))
            return PlaceholderSprite;

        return AvailableItems[Type];
    }

    public NumberedIconScreen GetVisualsForNumberedIcon(RectTransform GroupTransform, int i)
    {
        int Width = NumberedIconScreenWidth;
        GameObject ProductionUnit = Instantiate(NumberedIconPrefab);
        RectTransform Rect = ProductionUnit.GetComponent<RectTransform>();
        Rect.SetParent(GroupTransform, false);
        Rect.localPosition = new(i * Width, 0, 0);
        NumberedIconScreen UnitScreen = ProductionUnit.GetComponent<NumberedIconScreen>();
        return UnitScreen;
    }

    public GameObject GetVisualsForSetting(Setting Setting)
    {
        switch (Setting._Type)
        {
            case Setting.Type.Boolean: return Instantiate(BooleanSettingPrefab);
            case Setting.Type.Int: return Instantiate(IntSettingPrefab);
        }
        return null;
    }

    public GameObject GetVisualsForAbility(Transform Parent, GameplayAbility Ability)
    {
        GameObject Indicator = Instantiate(AbilitySlotPrefab);
        Indicator.transform.SetParent(Parent);
        return Indicator;
    }

    protected override void StartServiceInternal()
    {
        Refresh();
        _OnInit?.Invoke(this);
    }

    protected override void ResetInternal()
    {
        //todo: actually clear all GOs
    }

    protected override void StopServiceInternal() { }

    private static int NumberedIconScreenWidth = 62;
}
