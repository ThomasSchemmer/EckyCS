using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(BSpline))]
public class BSplineEdtior : Editor
{
    public override void OnInspectorGUI()
    {
        BSpline Target = target as BSpline;
        EditorGUI.BeginChangeCheck();
        base.OnInspectorGUI();
        Target.t = EditorGUILayout.Slider(Target.t, 0, Target.GetMaxIndex() - BCurve.NormalOffset);
        bool bIsPressed = EditorGUILayout.DropdownButton(new("Generate"), FocusType.Passive);
        bool bIsChanged = EditorGUI.EndChangeCheck();

        if (EditorGUILayout.DropdownButton(new("Convert"), FocusType.Passive))
        {
            Target.Convert();
        }
        if (!bIsPressed && !bIsChanged)
            return;

        Target.Generate();
        EditorUtility.SetDirty(target);
    }
}
