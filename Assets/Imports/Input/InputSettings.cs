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

    // maps to unity's implicit MouseInput scheme 
    public enum MouseInput  {
        LeftButton = 0,
        RightButton = 1,
        MiddleButton = 2,
    }

    // uses AxisNames to map int -> string
    public enum AxisInput
    {
        Horizontal = 0,
        Vertical = 1
    }

    public enum KeyInput
    {
        Key1 = KeyCode.Alpha1,
        Key2 = KeyCode.Alpha2,
        Key3 = KeyCode.Alpha3,
        Key4 = KeyCode.Alpha4,
    }


    public SerializedDictionary<Inputs, InputType> InputMap = new();

    public int[] KeyBindings = new int[Enum.GetValues(typeof(Inputs)).Length];

    public static string[] AxisNames =
    {
        "Horizontal",
        "Vertical"
    };
}
