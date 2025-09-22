using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

/**
 * Helper class to generate/find the scriptable objects for building/tile types
 */ 
public abstract class MeshFactory : GameService
{
    public Mesh UnknownMesh;

    protected abstract void LoadMeshes();

    public void Refresh()
    {
        LoadMeshes();
    }

    protected override void StartServiceInternal()
    {
        Refresh();
        _OnInit?.Invoke(this);
    }

    protected override void StopServiceInternal() {}




    protected override void ResetInternal()
    {
        //todo: release all meshes
    }
}
