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
        if (GUILayout.Button("BaseMesh", GUILayout.MaxWidth(100)))
        {
            Manager.GenerateDefaultMesh();
        }
        if (GUILayout.Button("HeightMesh", GUILayout.MaxWidth(100)))
        {
            Manager.GenerateMesh();
        }
        if (GUILayout.Button("Save", GUILayout.MaxWidth(100)))
        {
            Manager.SaveHeightTex();
        }
        if (GUILayout.Button("Reset", GUILayout.MaxWidth(100)))
        {
            Manager.Reset();
        }
        if (GUILayout.Button("Pixel", GUILayout.MaxWidth(100)))
        {
            Manager.Pixel();
        }
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        if (Manager.HeightRT != null)
        {
            Rect Rect = EditorGUILayout.GetControlRect(false, 128, GUILayout.MaxWidth(128));
            EditorGUI.DrawPreviewTexture(Rect, Manager.HeightRT);
        }
        if (Manager.PreviewRT != null)
        {
            Rect Rect = EditorGUILayout.GetControlRect(false, 128, GUILayout.MaxWidth(128));
            EditorGUI.DrawPreviewTexture(Rect, Manager.PreviewRT);
        }
        EditorGUILayout.EndHorizontal();
    }


}