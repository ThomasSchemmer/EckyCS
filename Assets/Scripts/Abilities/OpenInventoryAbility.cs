using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "OpenInventory", menuName = "ScriptableObjects/Abilities/Util/Inventory", order = 0)]
public class OpenInventoryAbility : GameplayAbility
{
    private InventoryService Inventory;

    public override void Activate()
    {
        base.Activate();
        if (Inventory == null)
            return;

        Inventory.ToggleScreen();
    }

    public override void OnGranted()
    {
        base.OnGranted();
        Game.RunAfterServiceInit((InventoryService Inv) =>
        {
            Inventory = Inv;
        });
    }

}
