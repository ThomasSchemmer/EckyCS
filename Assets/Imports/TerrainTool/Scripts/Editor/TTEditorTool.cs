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


    public void OnSceneGUI(SceneView View)
    {
        if (!bIsActive)
            return;

        if (Event.current.type == EventType.MouseDown && Event.current.button == 0)
        {
            Manager.Brush(View);
        }
        Manager.OnSceneGUI(View);
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
