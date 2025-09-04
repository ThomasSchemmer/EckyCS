using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Profiling;

public class EntityGenerator 
{
    public static bool TryCreate<T>(out T Entity, byte[] Data = null) where T : Entity, new()
    {
        Profiler.BeginSample("ECS.EntityGen.TryCreate");
        Entity = new T();
        if (!Game.TryGetService(out EckyCS ECS))
            return false;

        Entity.ID = new(CurrentID++);

        Type Type = typeof(T);
        ComponentGroupIdentifier GroupID = default;
        foreach (var Attribute in Type.GetCustomAttributes(typeof(RequireComponentTypeAttribute), true))
        {
            var RequiredType = Attribute as RequireComponentTypeAttribute;
            GroupID = RequiredType.ID;
        }
        ECS.RegisterEntity(Entity.ID, GroupID, Data);
        Profiler.EndSample();
        return true;
    }

    public static int GetSize<T>()
    {
        Profiler.BeginSample("ECS.EntityGen.GetSize");
        Type Type = typeof(T);
        int Length = 0;
        foreach (var Attribute in Type.GetCustomAttributes(typeof(RequireComponentTypeAttribute), true))
        {
            var RequiredType = Attribute as RequireComponentTypeAttribute;
            var ContainedTypes = RequiredType.ID.GetContainedTypes();
            foreach(var ContainedType in ContainedTypes)
            {
                Length += ComponentAllocator.GetSize(ContainedType);
            }
        }
        Profiler.EndSample();
        return Length;
    }

    public static int GetOffsetOf<T>(Type Type)
    {
        Profiler.BeginSample("ECS.EntityGen.GetSize");
        Type ClassType = typeof(T);
        int Offset = 0;
        foreach (var Attribute in ClassType.GetCustomAttributes(typeof(RequireComponentTypeAttribute), true))
        {
            var RequiredType = Attribute as RequireComponentTypeAttribute;
            var ContainedTypes = RequiredType.ID.GetContainedTypes();
            foreach (var ContainedType in ContainedTypes)
            {
                if (ContainedType != Type)
                {
                    Offset += ComponentAllocator.GetSize(ContainedType);
                }
                else{
                    return Offset;
                }
            }
        }
        Profiler.EndSample();
        return -1;
    }


    private static int CurrentID = 5;
}
