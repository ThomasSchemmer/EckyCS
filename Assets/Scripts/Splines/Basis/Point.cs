using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public class Point
{
    public Vector3 Position = new();

    public Point(Vector3 Position)
    {
        this.Position = Position;
    }

    public Point() { }
}
