using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public class PlantInfo : RenderInfo
{
    public int GrowthTimeS;
    public GrowthComponent.PlantType PlantType;
}
