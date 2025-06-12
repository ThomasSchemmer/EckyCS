using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

/** 
 * Wrapper for the BitVector indicating which components are represented
 * Adds utility for ComponentType <-> byte conversion
 */
public struct ComponentGroupIdentifier : IEnumerator, IEnumerable
{
    private BitVector Flags;
    // invalid == 0, as structs has no guaranteed init
    private int EnumeratorIndex;

    public ComponentGroupIdentifier(BitVector Flags)
    {
        this.Flags = Flags;
        EnumeratorIndex = 0;
    }

    public void AddFlag(Type Component)
    {
        int Index = ComponentAllocator.GetIDFor(Component);
        Flags.Set(Index, true);
    }

    public void AddFlags(Type[] Components)
    {
        foreach (Type Component in Components)
        {
            int Index = ComponentAllocator.GetIDFor(Component);
            Flags.Set(Index, true);
        }
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
        ComponentGroupIdentifier Clone = new(Flags.Clone());
        return Clone;
    }

    public ComponentGroupIdentifier Subtract(ComponentGroupIdentifier Other)
    {
        return new(Flags.Subtract(Other.Flags));
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

    /** returns the index of the requested type, ignoring all empty types */
    public int GetSelfIndexOf(Type Type)
    {
        return Flags.GetSelfIndexOf(Type);
    }
    public int[] GetSelfIndexOf(Type[] Types)
    {
        return Flags.GetSelfIndexOf(Types);
    }

    public List<Type> GetContainedTypes()
    {
        List<Type> Result = new();
        if (GetAmountOfFlags() == 0)
            return Result;

        foreach(Type Type in this)
        {
            Result.Add(Type);
        }
        return Result;
    }

    public bool MoveNext()
    {
        EnumeratorIndex = Flags.GetFirst(EnumeratorIndex, true) + 1;
        return EnumeratorIndex != 0;
    }

    public void Reset()
    {
        EnumeratorIndex = 0;
    }

    public IEnumerator GetEnumerator()
    {
        return this;
    }

    public object Current => ComponentAllocator.GetTypeFor(EnumeratorIndex - 1);
}
