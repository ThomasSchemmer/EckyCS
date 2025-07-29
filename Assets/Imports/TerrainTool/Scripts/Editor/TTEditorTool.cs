using Codice.Client.Common;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEditor.EditorTools;
using UnityEngine;

[EditorTool("Terrain Tool", typeof(TTManager))]
public class TTEditorTool : EditorTool, IDrawSelectedHandles
{
    private bool bIsActive = false;
    private TTManager Manager;

    public override void OnToolGUI(EditorWindow window)
    {
        if (window is not SceneView sceneView || Manager == null)
            return;

        HandleUtility.AddDefaultControl(GUIUtility.GetControlID(FocusType.Passive));

        Handles.BeginGUI();
        using (new GUILayout.HorizontalScope())
        {
            using (new GUILayout.VerticalScope(EditorStyles.helpBox))
            {
                sceneView.sceneViewState.alwaysRefresh = bIsActive;
                if (bIsActive && !sceneView.sceneViewState.fxEnabled)
                    sceneView.sceneViewState.fxEnabled = true;

                if (GUILayout.Button("Test"))
                {
                    Debug.Log("Lel");
                }
                EditorGUILayout.PrefixLabel("Brushsize ");
                Manager.BrushSize = EditorGUILayout.Slider(Manager.BrushSize, 0, Manager.Settings.Width, GUILayout.MaxWidth(128));
            }

            GUILayout.FlexibleSpace();
        }
        Handles.EndGUI();
    }


    public void OnDrawHandles()
    {
        //
        //Handles.PositionHandle(Manager.transform.position, Manager.transform.rotation);
    }
    //SceneView.lastActiveSceneView.ShowNotification(new GUIContent("Entering Terrain Tool"), .1f);

    Vector3 WorldStart;
    Vector2 ScreenStart;
    bool bIsDrawing = false;
    public void OnSceneGUI(SceneView View)
    {
        if (!bIsActive)
            return;

        if (Event.current.type == EventType.MouseDown && Event.current.button == 0)
        {
            WorldStart = Manager.GetMousePoint(View);
            ScreenStart = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
            bIsDrawing = true;
        }
        if (Event.current.type == EventType.MouseUp && Event.current.button == 0)
        {
            Manager.Brush(View, GetBrushStrength());
            bIsDrawing = false;
        }
        if (bIsDrawing) {
            Vector2 ScreenEnd = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
            float Strength = GetBrushStrength();
            Vector3 WorldEnd = WorldStart;
            WorldEnd.y += Strength;

            Handles.DrawDottedLine(WorldStart, WorldEnd, 8);
            Handles.BeginGUI();
            GUI.color = Color.black;
            Vector2 Temp = new(ScreenEnd.x, Screen.height - ScreenEnd.y);
            GUI.Label(new Rect(Temp, new(100, 25)), "" + Strength);
            Handles.EndGUI();
        }
        Manager.OnSceneGUI(View);
    }

    private float GetBrushStrength()
    {
        Vector2 ScreenEnd = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
        float Distance = (int)((ScreenEnd.y - ScreenStart.y) * 10) / 500.0f;
        Distance = Mathf.Clamp(Distance, -1, 1);
        return Distance;
    }

    public override void OnActivated()
    {
        if (target is not TTManager Manager)
            return;

        this.Manager = Manager;
        bIsActive = true;
        SceneView.duringSceneGui += OnSceneGUI;
    }

    public override void OnWillBeDeactivated()
    {
        Manager = null;
        SceneView.duringSceneGui -= OnSceneGUI;
    }
}
