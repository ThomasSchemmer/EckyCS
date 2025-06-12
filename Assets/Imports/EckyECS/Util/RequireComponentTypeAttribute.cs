using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[AttributeUsage(AttributeTargets.Class)]
public class RequireComponentTypeAttribute : System.Attribute
{
    public ComponentGroupIdentifier ID;

    public RequireComponentTypeAttribute(params Type[] Types)
    {
        ID = new();
        ID.AddFlags(Types);
    }
}
