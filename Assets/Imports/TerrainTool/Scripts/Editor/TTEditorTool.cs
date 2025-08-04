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

    private Texture2D BrushSoftTex, BrushHardTex;
    private TTBrushSettings Brush;

    Vector2 ScreenStart;
    bool bIsDrawing = false;

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

                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.PrefixLabel("Brushsize ");
                Brush.Size = EditorGUILayout.Slider(Brush.Size, 0, Manager.Settings.TexSize.x, GUILayout.MaxWidth(128));
                EditorGUILayout.EndHorizontal();

                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.PrefixLabel("Brushtype ");
                const int ButtonSize = 25;
                if (GUILayout.Button(BrushSoftTex, GUILayout.Width(ButtonSize), GUILayout.Height(ButtonSize)))
                {
                    Brush.Type = 0;
                }
                if (GUILayout.Button(BrushHardTex, GUILayout.Width(ButtonSize), GUILayout.Height(ButtonSize)))
                {
                    Brush.Type = 1;
                }
                EditorGUILayout.EndHorizontal();

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

    public void OnSceneGUI(SceneView View)
    {
        if (!bIsActive)
            return;

        if (Event.current.type == EventType.MouseDown && Event.current.button == 0)
        {
            Brush.WorldPos = Manager.GetMousePoint(View);
            ScreenStart = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
            bIsDrawing = true;
        }
        if (Event.current.type == EventType.MouseUp && Event.current.button == 0)
        {
            Manager.Brush(Brush);
            bIsDrawing = false;
        }
        if (bIsDrawing) {
            Vector2 ScreenEnd = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
            Brush.Strength = GetBrushStrength();
            Vector3 WorldEnd = Brush.WorldPos;
            WorldEnd.y += Brush.Strength * 10;

            Handles.DrawDottedLine(Brush.WorldPos, WorldEnd, 12);
            Handles.BeginGUI();
            GUI.color = Color.black;
            Vector2 Temp = new(ScreenEnd.x, Screen.height - ScreenEnd.y);
            GUI.Label(new Rect(Temp, new(100, 25)), "" + Brush.Strength);
            Handles.EndGUI();
        }
        Manager.OnSceneGUI(View, Brush);
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

        BrushSoftTex = Resources.Load("Textures/BrushSoft") as Texture2D;
        BrushHardTex = Resources.Load("Textures/BrushHard") as Texture2D;
    }

    public override void OnWillBeDeactivated()
    {
        Manager = null;
        SceneView.duringSceneGui -= OnSceneGUI;
    }
}
