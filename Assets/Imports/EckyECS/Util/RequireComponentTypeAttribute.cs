using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[AttributeUsage(AttributeTargets.Class)]
public class RequireComponentTypeAttribute : System.Attribute
{
    public Type Type;

    public RequireComponentTypeAttribute(Type Type)
    {
        this.Type = Type;
    }
}
