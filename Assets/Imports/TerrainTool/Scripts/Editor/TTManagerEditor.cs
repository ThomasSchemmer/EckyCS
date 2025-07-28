using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;


[CustomEditor(typeof(TTManager))]
public class TTManagerEditor : Editor
{
    public override void OnInspectorGUI()
    {
        base.OnInspectorGUI();

        TTManager Manager = (TTManager)target;

        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Generate", GUILayout.MaxWidth(100)))
        {
            Manager.Generate();
        }
        if (GUILayout.Button("Reset", GUILayout.MaxWidth(100)))
        {
            Manager.Reset();
        }
        if (GUILayout.Button("Debug", GUILayout.MaxWidth(100)))
        {
            Manager.Debug();
        }
        if (GUILayout.Button("Pixel", GUILayout.MaxWidth(100)))
        {
            Manager.Pixel();
        }
        EditorGUILayout.EndHorizontal();

        if (Manager.HeightRT != null)
        {
            Rect Rect = EditorGUILayout.GetControlRect(false, 128, GUILayout.MaxWidth(128));
            EditorGUI.DrawPreviewTexture(Rect, Manager.HeightRT);
        }
    }


}