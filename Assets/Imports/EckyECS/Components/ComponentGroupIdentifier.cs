using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

/** 
 * Wrapper for the BitVector indicating which components are represented
 * Adds utility for ComponentType <-> byte conversion
 */
public class ComponentGroupIdentifier : IEnumerator, IEnumerable
{
    private BitVector Flags;
    private int EnumeratorIndex = -1;


    public ComponentGroupIdentifier()
    {
        Flags = new(ComponentAllocator.MAX_NUM_COMPONENTS);
    }

    public void AddFlag(Type Component)
    {
        int Index = ComponentAllocator.GetIDFor(Component);
        Flags.Set(Index, true);
    }

    public bool HasFlag(Type Component)
    {
        int Index = ComponentAllocator.GetIDFor(Component);
        return Flags.Get(Index);
    }

    public void RemoveFlag(Type Component)
    {
        int Index = ComponentAllocator.GetIDFor(Component);
        Flags.Set(Index, false);
    }

    public bool HasAllFlags(List<Type> Components)
    {
        foreach (var Component in Components)
        {
            if (!HasFlag(Component))
                return false;
        }
        return true;
    }

    public bool HasAnyFlag(List<Type> Components)
    {
        foreach (var Component in Components)
        {
            if (HasFlag(Component))
                return true;
        }
        return false;
    }

    public int GetAmountOfFlags()
    {
        return Flags.GetAmountSetBits();
    }

    public ComponentGroupIdentifier Clone()
    {
        ComponentGroupIdentifier Clone = new();
        Clone.Flags = Flags.Clone();
        return Clone;
    }

    public ComponentGroupIdentifier Subtract(ComponentGroupIdentifier Other)
    {
        return new() {
            Flags = Flags.Subtract(Other.Flags)
        };
    }

    public override bool Equals(object obj)
    {
        if (obj is not ComponentGroupIdentifier Other)
            return false;

        return Flags.Equals(Other.Flags);
    }

    public override int GetHashCode()
    {
        return Flags.GetHashCode();
    }

    public override string ToString()
    {
        return Flags.ToString();
    }

    public List<Type> GetContainedTypes()
    {
        List<Type> Result = new();
        foreach(Type Type in this)
        {
            Result.Add(Type);
        }
        return Result;
    }

    public bool MoveNext()
    {
        EnumeratorIndex = Flags.GetFirst(EnumeratorIndex + 1, true);
        return EnumeratorIndex != -1;
    }

    public void Reset()
    {
        EnumeratorIndex = -1;
    }

    public IEnumerator GetEnumerator()
    {
        return this;
    }

    public object Current => ComponentAllocator.GetTypeFor(EnumeratorIndex);
}
