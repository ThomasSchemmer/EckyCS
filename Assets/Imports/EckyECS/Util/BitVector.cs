using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using UnityEngine.Assertions;

public class BitVector
{
    protected byte[] Data;

    public BitVector And(BitVector Other)
    {
        Assert.IsTrue(Data.Length == Other.Data.Length);
        BitVector NewVector = new(Data.Length);
        for (int i = 0; i < Data.Length; i++)
        {
            NewVector.Data[i] = (byte)(Data[i] & Other.Data[i]);
        }
        return NewVector;
    }

    public BitVector Or(BitVector Other)
    {
        Assert.IsTrue(Data.Length == Other.Data.Length);
        BitVector NewVector = new(Data.Length);
        for (int i = 0; i < Data.Length; i++)
        {
            NewVector.Data[i] = (byte)(Data[i] | Other.Data[i]);
        }
        return NewVector;
    }

    public BitVector Subtract(BitVector Other)
    {
        Assert.IsTrue(Data.Length == Other.Data.Length);
        BitVector NewVector = new(Data.Length, false);
        for (int i = 0; i < Data.Length; i++)
        {
            NewVector.Data[i] = (byte)Mathf.Max(Data[i] - Other.Data[i], 0);
        }
        return NewVector;
    }

    private bool IsEqual(BitVector Other)
    {
        Assert.IsTrue(Data.Length == Other.Data.Length);
        for (int i = 0; i < Data.Length; i++)
        {
            if (Data[i] != Other.Data[i])
                return false;
        }
        return true;
    }

    public BitVector(int Amount, bool bIsAmount = true)
    {
        int LengthInBytes = bIsAmount ? (int)Mathf.Log(Amount, 2) : Amount;
        Data = new byte[Mathf.CeilToInt(LengthInBytes)];
    }

    public bool Get(int Pos)
    {
        ToBytePos(Pos, out int PosByte, out int PosInByte);
        return GetByte(PosByte, PosInByte);
    }

    private bool GetByte(int PosByte, int PosInByte)
    {
        return (byte)(Data[PosByte] & (1 << PosInByte)) > 0;
    }

    public void Set(int Pos, bool Value)
    {
        ToBytePos(Pos, out int PosByte, out int PosInByte);
        if (Value)
        {
            Set(PosByte, PosInByte);
        }
        else
        {
            Clear(PosByte, PosInByte);
        }
    }

    private void Set(int PosByte, int PosInByte)
    {
        Data[PosByte] = (byte)(1 << PosInByte | Data[PosByte]);
    }

    private void Clear(int PosByte, int PosInByte)
    {
        Data[PosByte] = (byte)(0 << PosInByte & Data[PosByte]);
    }

    private void ToBytePos(int Pos, out int PosByte, out int PosInByte)
    {
        PosByte = Pos / 8;
        PosInByte = Pos - PosByte * 8;
    }

    public override string ToString()
    {
        StringBuilder SB = new(Data.Length * 8);
        for (int i = 0; i < Data.Length; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                string Var = GetByte(i, j) ? "1" : "0";
                SB.Append(Var);
            }
        }
        return SB.ToString();
    }

    public int GetFirstEmpty()
    {
        return GetFirst(0, false);
    }

    public int GetFirst(int Start, bool bShouldBeSet)
    {
        for (int i = Start; i < Data.Length; i++)
        {
            if (Data[i] == 0xFF)
                continue;

            for (int j = 0; j < 8; j++)
            {
                byte Temp = (byte)(Data[i] & (1 << j));
                byte Target = (byte)(bShouldBeSet ? 1 : 0);
                if (Temp != Target)
                    continue;

                return i * 8 + j;
            }
        }
        return -1;
    }

    // see https://graphics.stanford.edu/~seander/bithacks.html
    public int GetAmountSetBits()
    {
        int BitAmount = 0;
        for (int i = 0; i < Data.Length / 4; i++)
        {
            int Ai = i;
            int Bi = i + 1;
            int Ci = i + 2;
            int Di = i + 3;
            byte A = Ai < Data.Length ? Data[Ai] : (byte)0;
            byte B = Bi < Data.Length ? Data[Bi] : (byte)0;
            byte C = Ci < Data.Length ? Data[Ci] : (byte)0;
            byte D = Di < Data.Length ? Data[Di] : (byte)0;
            int Copy = A << 24 | B << 16 | C << 8 | D;
            Copy = Copy - ((Copy >> 1) & 0x55555555);
            Copy = (Copy & 0x33333333) + ((Copy >> 2) & 0x33333333);
            BitAmount += ((Copy + (Copy >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
        }

        return (int)BitAmount;
    }

    public int GetSize()
    {
        return Data.Length * 8;
    }

    public BitVector Clone()
    {
        BitVector Clone = new(Data.Length, false);
        Clone.Data = (byte[])Data.Clone();
        return Clone;
    }

    public override bool Equals(object obj)
    {
        if (obj is not BitVector Other)
            return false;

        return IsEqual(Other);
    }
    public override int GetHashCode()
    {
        int Code = 0;
        foreach (var Value in Data) {  Code += Value.GetHashCode(); }
        return Code;
    }

}
