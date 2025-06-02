using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(BezierSpline))]
public class BezierSplineEdtior : Editor
{
    public override void OnInspectorGUI()
    {
        BezierSpline Target = target as BezierSpline;
        EditorGUI.BeginChangeCheck();
        base.OnInspectorGUI();
        Target.t = EditorGUILayout.Slider(Target.t, 0, Target.GetMaxIndex() - BCurve.NormalOffset);
        bool bIsPressed = EditorGUILayout.DropdownButton(new("Generate"), FocusType.Passive);
        bool bIsChanged = EditorGUI.EndChangeCheck();

        if (!bIsPressed && !bIsChanged)
            return;

        Target.Generate();
        EditorUtility.SetDirty(target);
    }
}
