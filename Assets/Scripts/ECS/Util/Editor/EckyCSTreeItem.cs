using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor.IMGUI.Controls;
using UnityEngine;

[Serializable]
public class EckyCSTreeItem : TreeViewItem
{
    public string Comp0, Comp1, Comp2, Comp3;

    public EckyCSTreeItem(int id)
    {
        this.id = id;

    }
}