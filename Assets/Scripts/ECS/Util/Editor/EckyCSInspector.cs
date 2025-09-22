using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEditor.IMGUI.Controls;
using UnityEditor.PackageManager.UI;
using UnityEngine;
using UnityEngine.UIElements;

public class EckyCSInspector : EditorWindow
{
    [SerializeField] TreeViewState TreeViewState;
    EckyCSTreeView TreeView;

    [MenuItem("Window/EckyCS/Inspector")]
    public static void ShowMyEditor()
    {
        EditorWindow wnd = GetWindow<EckyCSInspector>();
        wnd.titleContent = new GUIContent("EckyCS Inspector");

        wnd.minSize = new Vector2(450, 200);
        wnd.maxSize = new Vector2(1920, 720);

        wnd.autoRepaintOnSceneChange = true;

    }

    void OnEnable()
    {
        if (TreeViewState == null)
            TreeViewState = new TreeViewState();

        TreeView = new EckyCSTreeView(TreeViewState);
    }

    private void OnInspectorUpdate()
    {
        if (TreeView == null)
            return;

        TreeView.Refresh();
        Repaint(); 
    }

    void OnGUI()
    {
        if (!EditorApplication.isPlaying)
        {
            EditorGUILayout.LabelField("Press play to view entities");
            return;
        }
        TreeView.OnGUI(new(new(), position.size));
    }

}
