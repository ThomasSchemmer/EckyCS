using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public struct HighlightComponent : IComponent
{
    // could be a bool but leads to problems with parsing to GPU
    public uint IsHighlighted;
}
