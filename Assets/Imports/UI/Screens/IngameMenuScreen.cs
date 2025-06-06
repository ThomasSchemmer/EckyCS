using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

/**
 * Represents the ingame menu (escape)
 */
public class IngameMenuScreen : ScreenUI
{
    public void ButtonClick()
    {
        if (IsShown)
        {
            Hide();
        }
        else
        {
            Show();
        }
    }

    public void OnSave()
    {
        ConfirmScreen.Show("Please enter a savegame name", OnSaveConfirm, true);
    }

    private void OnSaveConfirm()
    {
        if (!Game.TryGetService(out SaveGameManager Manager))
            return;

        string FileName = SaveGameManager.GetCompleteSaveGameName(ConfirmScreen.GetInputText());
        Manager.Save(false, FileName);
        MessageSystemScreen.CreateMessage(Message.Type.Success, "Saved successfully");
        Hide();
    }

    public override void Show()
    {
        base.Show();
        _OnOpenBegin?.Invoke();
        IsShown = true;
        _OnOpenFinish?.Invoke();
    }

    public override void Hide()
    {
        base.Hide();
        IsShown = false;
        _OnClose?.Invoke();
    }


    public void Update()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            ButtonClick();
        }
    }

    public void OnEnable()
    {
        Instance = this;
    }

    public void OnClickExit()
    {
        Action A = () =>
        {
            Game.LoadGame(null, Game.MenuSceneName);
        };
        ConfirmScreen.Show("Are you sure you want exit the game? Any unsaved progress will be lost!", A);
    }

    public void OnClickQuit()
    {
        Action A = () =>
        {
            Game.QuitGame();
        };
        ConfirmScreen.Show("Are you sure you want quit the game? Any unsaved progress will be lost!", A);
    }

    private bool IsShown = false;

    public delegate void OnOpenBegin();
    public delegate void OnOpenFinish();
    public delegate void OnClose();
    public OnOpenBegin _OnOpenBegin;
    public OnOpenFinish _OnOpenFinish;
    public OnClose _OnClose;

    public static IngameMenuScreen Instance;
}
