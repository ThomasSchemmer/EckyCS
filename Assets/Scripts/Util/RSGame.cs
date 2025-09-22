using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RSGame : Game
{
    protected override void InitInternal()
    {
        base.InitInternal();
        if (IngameMenuScreen.Instance)
        {
            IngameMenuScreen.Instance._OnOpenBegin += OnOpenMenu;
            IngameMenuScreen.Instance._OnClose += OnCloseMenu;
        }
    }

    public override void GameOver(string Message = null)
    {
        base.GameOver(Message);
        GameOverScreen.GameOver(Message);
    }
}
