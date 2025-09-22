using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RSMeshFactory : MeshFactory
{

    public SerializedDictionary<AbilityType, Mesh> AbilityMeshes = new();

    protected override void LoadMeshes()
    {
        LoadAbilities();
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

}
