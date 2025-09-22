using System;
using System.Security.AccessControl;
using TMPro;
using Unity.VectorGraphics;
using UnityEngine;
using UnityEngine.UI;

public abstract class IconFactory : GameService
{
    protected Sprite PlaceholderSprite;

    public abstract void Refresh();


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

}
