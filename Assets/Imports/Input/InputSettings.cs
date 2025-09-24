using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "InputSettings", menuName = "ScriptableObjects/Input/Settings", order = 0)]
public class InputSettings : ScriptableObject
{
    public enum InputScheme
    {
        KeyboardMouse,
        Controller
    }

    public enum Inputs : int
    {
        INVALID = 0,
        Interact,
        MoveUp,
        MoveLeft,
        MoveDown,
        MoveRight,
        Ability0,
        Ability1, 
        Ability2, 
        Ability3
    }

    public enum InputType
    {
        Key,
        Axis,
        Mouse
    }

    public SerializedDictionary<Inputs, InputType> InputMap = new();

    public int[] Keybinds = new int[Enum.GetValues(typeof(Inputs)).Length];
    public string[] AxisNames = new string[Enum.GetValues(typeof(Inputs)).Length];
}
