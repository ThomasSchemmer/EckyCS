using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(InputSettings))]
public class InputSettingsInspector : Editor
{

    public override void OnInspectorGUI()
    {
        InputSettings Settings = target as InputSettings;
        EditorGUILayout.BeginVertical();
        EditorGUILayout.LabelField("Input types: ");
        InputSettings.Inputs[] Keys = new InputSettings.Inputs[Settings.InputMap.Count];
        Settings.InputMap.Keys.CopyTo(Keys, 0);
        foreach (var Key in Keys)
        {
            EditorGUILayout.BeginHorizontal();
            Settings.InputMap[Key] = (InputSettings.InputType)EditorGUILayout.EnumPopup(Key.ToString(), Settings.InputMap[Key]);
            DisplayInputEntry(Settings, Key);
            EditorGUILayout.EndHorizontal();
        }
        EditorGUILayout.EndVertical();
    }

    private void DisplayInputEntry(InputSettings Settings, InputSettings.Inputs Key)
    {
        switch (Settings.InputMap[Key])
        {
            case InputSettings.InputType.Mouse: DisplayMouseInput(Settings, Key); break;
            case InputSettings.InputType.Axis: DisplayAxisInput(Settings, Key); break;
            case InputSettings.InputType.Key: DisplayKeyInput(Settings, Key); break;
        }
    }

    private void DisplayMouseInput(InputSettings Settings, InputSettings.Inputs Key) {
        Settings.KeyBindings[(int)Key] = (int)(InputSettings.MouseInput)EditorGUILayout.EnumPopup((InputSettings.MouseInput)Settings.KeyBindings[(int)Key]);
    }
    private void DisplayAxisInput(InputSettings Settings, InputSettings.Inputs Key)
    {
        Settings.KeyBindings[(int)Key] = (int)(InputSettings.AxisInput)EditorGUILayout.EnumPopup((InputSettings.AxisInput)Settings.KeyBindings[(int)Key]);
    }
    private void DisplayKeyInput(InputSettings Settings, InputSettings.Inputs Key)
    {
        Settings.KeyBindings[(int)Key] = (int)(InputSettings.KeyInput)EditorGUILayout.EnumPopup((InputSettings.KeyInput)Settings.KeyBindings[(int)Key]);
    }
}
