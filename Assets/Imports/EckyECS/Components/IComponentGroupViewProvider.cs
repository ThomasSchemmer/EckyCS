using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Profiling;

public interface IComponentGroupViewProvider<T>
{
    public abstract Dictionary<ComponentGroupIdentifier, T> GetViewSet();

    public IComponentGroupViewProvider<T> GetProvider();

    public ComponentGroupView<X> Get<X>() where X : IComponent
    {
        Profiler.BeginSample("ICGVProvider.Get_X");
        ComponentGroupView<X> Groups = new();
        foreach (var Key in GetViewSet().Keys)
        {
            if (!Key.HasFlag(typeof(X)))
                continue;

            Groups.Add(Key);
        }
        Profiler.EndSample();
        return Groups;
    }

    public ComponentGroupView<X, Y> Get<X, Y>() where X : IComponent where Y : IComponent
    {
        Profiler.BeginSample("ICGVProvider.Get_X_Y");
        ComponentGroupView<X, Y> Groups = new();
        List<System.Type> Reqs = new()
        {
            typeof(X),
            typeof(Y)
        };
        foreach (var Key in GetViewSet().Keys)
        {
            if (!Key.HasAllFlags(Reqs))
                continue;

            Groups.Add(Key);
        }
        Profiler.EndSample();
        return Groups;
    }

}
