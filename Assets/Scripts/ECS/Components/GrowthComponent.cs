using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public struct GrowthComponent : IComponent
{
    public int Growth;
    public PlantType Plant;
    public int PlantedAtS;

    public enum PlantType
    {
        ANY,
        Corn
    }

}
