using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Profiling;

public class ComponentAllocator
{
    public static Dictionary<Type, int> TypeToID = new();
    public static Dictionary<int, Type> IDToType = new();
    public static Dictionary<Type, int> TypeToSize = new();


    public static int GetIDFor(Type Type)
    {
        Register(Type);
        return TypeToID[Type];
    }

    public static Type GetTypeFor(int ID)
    {
        Assert.IsTrue(IDToType.ContainsKey(ID));
        return IDToType[ID];
    }

    private static void Register(Type Type)
    {
        if (TypeToID.ContainsKey(Type))
            return;

        TypeToID.Add(Type, CURRENT_COMP_INDEX++);
        IDToType.Add(TypeToID[Type], Type);
        TypeToSize.Add(Type, GetSize(Type));
    }

    public static int GetSize(Type Type)
    {
        return Marshal.SizeOf(Type);
    }

    private static int CURRENT_COMP_INDEX = 0;
}
