using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static ItemComponent;
public class InventoryService : GameService
{
    public InventoryScreen Screen;

    private Dictionary<ItemType, int> ItemCounts = new();
    private List<ItemType> ItemOrder = new();

    public void AddItem(ItemType Item, int Amount)
    {
        if (!HasItem(Item))
        {
            CreateItem(Item);
        }
        ItemCounts[Item] += Amount;
    }

    public bool HasItem(ItemType Item)
    {
        return ItemCounts.ContainsKey(Item);
    }

    public bool HasEnoughOfItem(ItemType Item, int RequiredAmount)
    {
        return HasItem(Item) && ItemCounts[Item] >= RequiredAmount;
    }

    public bool TryGetItemInfoAt(int i, out ItemType Type, out int Amount)
    {
        Type = ItemType.DEFAULT;
        Amount = 0;
        if (i >= ItemOrder.Count)
            return false;

        Type = ItemOrder[i];
        Amount = ItemCounts[Type];
        return true;
    }

    public void SubtractItem(ItemType Item, int Amount)
    {
        if (!HasEnoughOfItem(Item, Amount))
            return;

        ItemCounts[Item] -= Amount;
        if (ItemCounts[Item] > 0)
            return;

        DeleteItem(Item);
    }

    public void ToggleScreen()
    {
        if (Screen == null)
            return;

        Screen.Toggle();
    }

    private void DeleteItem(ItemType Item)
    {
        ItemCounts.Remove(Item);
        ItemOrder.Remove(Item);
    }

    private void CreateItem(ItemType Item)
    {
        ItemCounts.Add(Item, 0);
        ItemOrder.Add(Item);
    }

    protected override void ResetInternal()
    {
        ItemCounts.Clear();
        ItemOrder.Clear();
    }

    protected override void StartServiceInternal()
    {
        _OnInit?.Invoke(this);
    }

    protected override void StopServiceInternal()
    {
    }
}
