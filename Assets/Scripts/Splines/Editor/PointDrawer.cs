using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[CustomPropertyDrawer(typeof(Point))]
public class PointDrawer : PropertyDrawer
{
    public override void OnGUI(Rect position, SerializedProperty Property, GUIContent label)
    {
        SerializedProperty PositionProp = Property.FindPropertyRelative("Position");
        EditorGUILayout.PropertyField(PositionProp);
    }
}
