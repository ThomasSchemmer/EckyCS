using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public struct TransformComponent : IComponent
{
    public float PosX;
    public float PosY;
    public float PosZ;

    public readonly Vector3 GetPosition()
    {
        return new(PosX, PosY, PosZ);
    }
}
