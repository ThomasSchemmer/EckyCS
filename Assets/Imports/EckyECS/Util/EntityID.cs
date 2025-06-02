using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public struct EntityID
{
    // split into 24bit ID and 8 bit versions, so max ID is 16.777.216
    private uint Data;


    public EntityID(int _ID, uint _Version = 0) : this()
    {
        ID = (int)_ID;
        Version = _Version;
    }

    public static int operator/ (EntityID Self, uint Other)
    {
        return (int)(Self.ID / Other);
    }

    public int ID {
        get {
            return (int)Data >> ID_OFFSET;
        } 
        private set
        {
            Data = (uint)(((value << ID_OFFSET) & ID_MASK) | (Data & VERSION_MASK));
        }
    }

    public uint Version
    {
        get
        {
            return Data & VERSION_MASK;
        }
        private set
        {
            Data = (value & VERSION_MASK) | (Data & ID_MASK);
        }
    }

    public override bool Equals(object obj)
    {
        if (obj is not EntityID Other)
            return false;

        return this.Data == Other.Data;
    }

    public override int GetHashCode()
    {
        return Data.GetHashCode();
    }

    public static EntityID Invalid(int ID)
    {
        return new EntityID(ID, INVALID);
    }
    public static EntityID Invalid()
    {
        return new EntityID(0, INVALID);
    }

    public override string ToString()
    {
        return Version == INVALID ? 
            "Invalid" : 
            "" + ID + " | " + Version;
    }

    public bool IsInvalid()
    {
        return Version == INVALID;
    }

    public void Invalidate()
    {
        Version = INVALID;
    }


    private static int ID_OFFSET = 8;
    private static uint ID_MASK = 0xFFFFFF00;
    private static uint VERSION_MASK = 0x000000FF;
    public static uint INVALID = (uint)(Mathf.Pow(2, 8) - 1);
}
