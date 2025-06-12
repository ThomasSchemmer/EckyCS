using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Profiling;

public class EntityGenerator 
{
    public static bool TryCreate<T>(out T Entity) where T : Entity, new()
    {
        Profiler.BeginSample("ECS.EntityGen.TryCreate");
        Entity = new T();
        if (!Game.TryGetService(out ECS ECS))
            return false;

        Entity.ID = new(CurrentID++);

        Type Type = typeof(T);
        ComponentGroupIdentifier GroupID = default;
        foreach (var Attribute in Type.GetCustomAttributes(typeof(RequireComponentTypeAttribute), true))
        {
            var RequiredType = Attribute as RequireComponentTypeAttribute;
            GroupID = RequiredType.ID;
        }
        ECS.RegisterEntity(Entity.ID, GroupID);
        Profiler.EndSample();
        return true;
    }


    private static int CurrentID = 0;
}
