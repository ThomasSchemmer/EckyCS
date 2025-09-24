using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static InputSettings;

public class InputManager : GameService
{
    protected List<InputSettings> Settings = new();

    public bool IsInputDown(int PlayerID, Inputs Type)
    {
        if (!HasPlayer(PlayerID) || Type == Inputs.INVALID) return false;

        var Setting = Settings[PlayerID];
        switch (Setting.InputMap[Type])
        {
            case InputType.Axis: return false;
            case InputType.Mouse: return Input.GetMouseButtonDown(Setting.Keybinds[(int)Type]);
            case InputType.Key: return Input.GetKeyDown((KeyCode)Setting.Keybinds[(int)Type]);
            default: return false;
        }
    }

    public float GetInput(int PlayerID, Inputs Type)
    {
        if (!HasPlayer(PlayerID)) return 0;

        var Setting = Settings[PlayerID];
        switch (Setting.InputMap[Type])
        {
            case InputType.Axis: return Input.GetAxis(Setting.AxisNames[(int)Type]);
            case InputType.Mouse: return Input.GetMouseButton(Setting.Keybinds[(int)Type]) ? 1 : 0;
            case InputType.Key: return Input.GetKey((KeyCode)Setting.Keybinds[(int)Type]) ? 1 : 0;
            default: return 0;
        }
    }

    public bool IsInputUp(int PlayerID, Inputs Type)
    {
        if (!HasPlayer(PlayerID)) return false;

        var Setting = Settings[PlayerID];
        switch (Setting.InputMap[Type])
        {
            case InputType.Axis: return false;
            case InputType.Mouse: return Input.GetMouseButtonUp(Setting.Keybinds[(int)Type]);
            case InputType.Key: return Input.GetKeyUp((KeyCode)Setting.Keybinds[(int)Type]);
            default: return false;
        }
    }

    public bool HasPlayer(int PlayerID)
    {
        // TODO: Controller support
        if (PlayerID == 1)
            return false;

        return PlayerID < Settings.Count && Settings[PlayerID] != null;
    }

    protected override void ResetInternal()
    {
    }

    protected override void StartServiceInternal()
    {
    }

    protected override void StopServiceInternal()
    {
    }

    protected const string SettingsLocation = "Player/InputSettings";
}
