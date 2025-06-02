using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

/**
 * Helper class to generate/find the scriptable objects for building/tile types
 */ 
public class MeshFactory : GameService
{
    public Mesh UnknownMesh;

    public SerializedDictionary<AbilityType, Mesh> AbilityMeshes = new();

    private void LoadMeshes()
    {
        LoadAbilities();
    }

    public void Refresh()
    {
        LoadMeshes();
    }


    private void LoadAbilities()
    {
        AbilityMeshes.Clear();
        var AbilityTypes = Enum.GetValues(typeof(AbilityType));
        foreach (var AbilityType in AbilityTypes)
        {
            GameObject GO = Resources.Load("Models/Abilities/" + AbilityType) as GameObject;
            if (!GO || !GO.TryGetComponent<MeshFilter>(out var Filter))
                continue;

            if (Filter.sharedMesh == null)
                continue;

            AbilityMeshes.Add((AbilityType)AbilityType, Filter.sharedMesh);
        }
    }
    
    public Mesh GetAbilityMesh(AbilityType Type)
    {
        if (!AbilityMeshes.ContainsKey(Type))
            return null;

        return AbilityMeshes[Type];
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
