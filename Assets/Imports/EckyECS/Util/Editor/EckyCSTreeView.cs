using System.Collections;
using System.Collections.Generic;
using UnityEditor.IMGUI.Controls;
using UnityEngine;

class EckyCSTreeView : TreeView
{
    public EckyCSTreeView(TreeViewState treeViewState)
        : base(treeViewState)
    {
        Reload();
        MultiColumnHeaderState.Column[] Columns = new MultiColumnHeaderState.Column[]
        {
            new(){maxWidth = 250}, new(){maxWidth = 250}
        };
        MultiColumnHeaderState State = new(Columns);
        this.multiColumnHeader = new(State);
    }

    protected override void RowGUI(RowGUIArgs args)
    {
        base.RowGUI(args);
        if (args.item is not EckyCSTreeItem EckyItem)
            return;

        for (int i = 0; i < args.GetNumVisibleColumns(); ++i)
        {
            CellGUI(args.GetCellRect(i), EckyItem, args.GetColumn(i), ref args);
        }
    }

    void CellGUI(Rect cellRect, EckyCSTreeItem item, int column, ref RowGUIArgs args)
    {
        CenterRectUsingSingleLineHeight(ref cellRect);

        GUI.Label(cellRect, item.Comp0);
    }

    public void Refresh()
    {
        Reload();
    }

    int ID;
    List<TreeViewItem> AllItems;

    private unsafe void LoadECSList()
    {
        if (!Game.TryGetService(out ECS ECS))
            return;

        ID = 0;
        foreach (var Tuple in ECS.EntitySets)
        {
            if (Tuple.Key.GetAmountOfFlags() == 0)
                continue;

            Register(
                Tuple.Key, 
                Tuple.Value.GetGroupPointers(), 
                Tuple.Value.GetCount()
            );
        }
    }

    private unsafe void Register(ComponentGroupIdentifier Group, void*[] Ptrs, int Count)
    {
        int Target = Group.GetSelfIndexOf(typeof(TransformComponent));
        if (Target < 0)
            return;

        TransformComponent* Ptr = (TransformComponent*)Ptrs[Target];
        for (int i = 0; i < Count; i++) {
            EckyCSTreeItem Item = new(ID++);
            Item.Comp0 = (Ptr + i)->GetPosition().ToString();
            AllItems.Add(Item);
        }
    }

    protected override TreeViewItem BuildRoot()
    {
        var root = new TreeViewItem { id = 0, depth = -1, displayName = "Root" };
        AllItems = new List<TreeViewItem>();

        LoadECSList();

        if (AllItems.Count == 0)
        {
            AllItems.Add(new TreeViewItem(0) { id = 0, depth = 0, displayName = "Nothing to display" });
        }

        SetupParentsAndChildrenFromDepths(root, AllItems);

        return root;
    }
}