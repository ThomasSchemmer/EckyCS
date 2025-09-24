using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RSInputManager : InputManager
{
    private void InitKeybinds(PlayerInstantiatorService Players)
    {
        var BaseSettings = Resources.Load(SettingsLocation) as InputSettings;
        for (int i = 0; i < Players.GetPlayerCount(); i++)
        {
            Settings.Add(Instantiate(BaseSettings));
        }
    }
    protected override void StartServiceInternal()
    {
        Game.RunAfterServiceInit((PlayerInstantiatorService Players) =>
        {
            InitKeybinds(Players);
        });
    }
}
