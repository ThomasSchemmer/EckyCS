using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public class PlantInfo : RenderInfo
{
    public Mesh Mesh;
    public int GrowthTimeS;
    public GrowthComponent.PlantType PlantType;

    public override Mesh GetMesh()
    {
        return Mesh;
    }
}
