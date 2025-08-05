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
                EditorGUILayout.LabelField("Brush: ");
                EditorGUILayout.EndHorizontal();

                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.PrefixLabel("Size ");
                Brush.Size = EditorGUILayout.Slider(Brush.Size, 0, GetMaxBrushSize(), GUILayout.MaxWidth(128));
                EditorGUILayout.EndHorizontal();

                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.PrefixLabel("Type ");
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


                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.PrefixLabel("OverrideType ");
                if (GUILayout.Button("Add", GUILayout.Width(ButtonSize * 2), GUILayout.Height(ButtonSize)))
                {
                    Brush.OverrideType = 0;
                }
                if (GUILayout.Button("Max", GUILayout.Width(ButtonSize * 2), GUILayout.Height(ButtonSize)))
                {
                    Brush.OverrideType = 1;
                }
                EditorGUILayout.EndHorizontal();

                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.PrefixLabel("Preview ");
                bool bOld = Brush.bIsPreview;
                Brush.bIsPreview = GUILayout.Toggle(Brush.bIsPreview, "");
                if (Brush.bIsPreview && !bOld)
                {
                    Manager.ResetRT(true);
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
            Brush.CurrentWorldPos = Manager.ProjectMouseOnPlane(View);
            Brush.StartWorldPos = Brush.CurrentWorldPos;
            ScreenStart = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
            bIsDrawing = true;
        }
        if (Event.current.type == EventType.MouseUp && Event.current.button == 0)
        {
            Manager.Brush(Brush);
            bIsDrawing = false;
        }
        CheckPreview();
        if (bIsDrawing) {
            if (Brush.bIsPreview)
            {
                HandlePreview(View);
            }
            else {
                HandleBrushing();
            }
        }

        if (Event.current.type == EventType.ScrollWheel && Event.current.shift)
        {
            Brush.Size += Event.current.delta.x;
            Brush.Size = Mathf.Clamp(Brush.Size, 0, GetMaxBrushSize());
        }
        Manager.OnSceneGUI(View, Brush);
    }

    private void CheckPreview()
    {
        if (Event.current.keyCode != KeyCode.LeftShift)
            return;

        bool bOld = Brush.bIsPreview;
        Brush.bIsPreview =
            Event.current.type != EventType.KeyUp && ((Event.current.type == EventType.KeyDown) || Brush.bIsPreview);

        if (Brush.bIsPreview && !bOld)
        {
            Manager.ResetRT(true);
        }
    }

    private void HandlePreview(SceneView View)
    {
        Brush.CurrentWorldPos = Manager.ProjectMouseOnPlane(View);
        Brush.Strength = 1;
        Manager.Brush(Brush);
    }

    private void HandleBrushing()
    {
        Vector2 ScreenEnd = HandleUtility.GUIPointToScreenPixelCoordinate(Event.current.mousePosition);
        Brush.Strength = GetBrushStrength();
        Vector3 WorldEnd = Brush.CurrentWorldPos;
        WorldEnd.y += Brush.Strength * 10;

        Handles.DrawDottedLine(Brush.CurrentWorldPos, WorldEnd, 12);
        Handles.BeginGUI();
        GUI.color = Color.black;
        Vector2 Temp = new(ScreenEnd.x, Screen.height - ScreenEnd.y);
        GUI.Label(new Rect(Temp, new(100, 25)), "" + Brush.Strength);
        Handles.EndGUI();
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

    private float GetMaxBrushSize(){
        return Manager.Settings.TexSize.x / 5.0f;
    }
}
