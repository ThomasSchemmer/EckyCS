using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
[CreateAssetMenu(fileName = "TTSettings", menuName = "TerrainTool/Settings", order = 0)]
public class TTSettings : ScriptableObject
{
    public int Width, Height;
}
