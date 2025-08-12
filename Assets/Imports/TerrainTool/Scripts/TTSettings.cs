using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
[CreateAssetMenu(fileName = "TTSettings", menuName = "TerrainTool/Settings", order = 0)]
public class TTSettings : ScriptableObject
{
    [SerializeField]
    public Vector2Int TexSize = new(128, 128);

    [SerializeField]
    public Vector3Int WorldSize = new(1, 1, 1);

    public int Bands = 10;
}
