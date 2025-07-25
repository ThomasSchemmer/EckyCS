using System.Collections;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(BSpline)), CanEditMultipleObjects]
public class BSplineEdtior : Editor
{
    BSpline Target;
    List<Matrix4x4> Matrices = new();

    public override void OnInspectorGUI()
    {
        Target = target as BSpline;
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


    protected virtual void OnSceneGUI()
    {
        Target = target as BSpline;

        List<int> ToDelete = new();
        for (int i = 0; i < Target.Points.Count; i++)
        {
            Vector3 Position = Target.Points[i].Position;
            Target.Points[i].Position = Handles.PositionHandle(Position, Quaternion.identity);
            if (Handles.Button(
                Position + new Vector3(0, 0, -1),
                Quaternion.identity,
                HandleUtility.GetHandleSize(Position) * 2,
                HandleUtility.GetHandleSize(Position) * 2,
                DeleteButtonCap))
            {
                ToDelete.Add(i);
            }
        }

        if (Event.current.type == EventType.MouseUp)
        {
            for (int i = ToDelete.Count - 1; i >= 0; i++)
            {
                Target.Points.RemoveAt(ToDelete[i]);
            }
        }

        DrawButtons();
    }

    private void DrawButtons()
    {
        Graphics.DrawMeshInstanced(Target.Mesh, 0, Target.Mat, Matrices);
        Matrices.Clear();
    }

    private void DeleteButtonCap(int controlID, Vector3 position, Quaternion rotation, float size, EventType eventType)
    {
        
        switch (eventType)
        {
            case EventType.MouseMove:
            case EventType.Layout:
                HandleUtility.AddControl(controlID, HandleUtility.DistanceToCircle(position, size));
                break;
            case EventType.Repaint:
                Matrices.Add(Matrix4x4.TRS(position, rotation, Vector3.one));
                break;
        }
    }
}
