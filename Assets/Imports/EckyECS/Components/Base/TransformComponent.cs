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
    public readonly Vector2 GetPositionXZ()
    {
        return new(PosX, PosZ);
    }
    public void SetPosition(Vector3 Pos)
    {
        PosX = Pos.x;
        PosY = Pos.y;
        PosZ = Pos.z;
    }

}
