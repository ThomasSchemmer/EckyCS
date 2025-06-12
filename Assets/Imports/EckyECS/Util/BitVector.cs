using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using UnityEngine.Assertions;

[StructLayout(LayoutKind.Sequential, Pack = 1)]
/** 
 * "string" of bits indicating wheether something is represented by this vector or not
 * 
 * Uses multiple ints as byte[] is for some reason still managed, even with MarshalAs. See
 * see https://limbioliong.wordpress.com/2011/06/29/passing-managed-structures-with-arrays-to-unmanaged-code-part-1/
 * for more info
 */
public unsafe struct BitVector
{
    private const int IntCount = 4;
    private const int IntSize = 32;

    // since mem layout is sequential we can use ptrs here
    // its still not pretty though
    private int Data0;
    private int Data1;
    private int Data2;
    private int Data3;

    public readonly BitVector And(BitVector Other)
    {
        BitVector NewVector = new();
        NewVector.Data0 = (Data0 & Other.Data0); 
        NewVector.Data1 = (Data1 & Other.Data1); 
        NewVector.Data2 = (Data2 & Other.Data2); 
        NewVector.Data3 = (Data3 & Other.Data3); 
        return NewVector;
    }

    public readonly BitVector Or(BitVector Other)
    {
        BitVector NewVector = new();
        NewVector.Data0 = (Data0 | Other.Data0);
        NewVector.Data1 = (Data1 | Other.Data1);
        NewVector.Data2 = (Data2 | Other.Data2);
        NewVector.Data3 = (Data3 | Other.Data3);
        return NewVector;
    }

    public readonly BitVector Subtract(BitVector Other)
    {

        BitVector NewVector;
        NewVector.Data0 = Mathf.Max(Data0 - Other.Data0, 0);
        NewVector.Data1 = Mathf.Max(Data1 - Other.Data1, 0);
        NewVector.Data2 = Mathf.Max(Data2 - Other.Data2, 0);
        NewVector.Data3 = Mathf.Max(Data3 - Other.Data3, 0);
        return NewVector;
    }

    private readonly bool IsEqual(BitVector Other)
    {
        return
            Data0 == Other.Data0 &&
            Data1 == Other.Data1 &&
            Data2 == Other.Data2 &&
            Data3 == Other.Data3;
    }


    public readonly bool Get(int Pos)
    {
        ToBitPos(Pos, out int PosInt, out int PosInInt);
        return GetBit(PosInt, PosInInt);
    }

    private readonly bool GetBit(int PosInt, int PosInInt)
    {
        fixed (int* IntPtr = &Data0)
        {
            return (IntPtr[PosInt] & (1 << PosInInt)) > 0;
        }
    }

    public void Set(int Pos, bool Value)
    {
        ToBitPos(Pos, out int PosByte, out int PosInByte);
        if (Value)
        {
            Set(PosByte, PosInByte);
        }
        else
        {
            Clear(PosByte, PosInByte);
        }
    }

    private void Set(int PosInt, int PosInInt)
    {
        fixed (int* IntPtr = &Data0)
        {
            IntPtr[PosInt] = (1 << PosInInt | IntPtr[PosInt]);
        }
    }

    private void Clear(int PosInt, int PosInInt)
    {
        fixed (int* IntPtr = &Data0)
        {
            IntPtr[PosInt] = (0 << PosInInt & IntPtr[PosInt]);
        }
    }

    private readonly void ToBitPos(int Pos, out int PosInt, out int PosInInt)
    {
        PosInt = Pos / IntSize;
        PosInInt = Pos - PosInt * IntSize;
    }

    public readonly int GetSelfIndexOf(Type Type)
    {
        int TargetIndex = ComponentAllocator.GetIDFor(Type);
        if (!Get(TargetIndex))
            return -1;

        int Count = 0;
        for (int i = 0; i < TargetIndex; i++)
        {
            Count += Get(i) ? 1 : 0;
        }
        return Count;
    }

    public readonly int[] GetSelfIndexOf(Type[] Types)
    {
        List<int> Result = new();
        foreach (var Type in Types)
        {
            int Value = GetSelfIndexOf(Type);
            if (Value == -1)
                continue;

            Result.Add(Value);
        }
        return Result.ToArray();
    }

    public readonly override string ToString()
    {
        StringBuilder SB = new(IntCount * IntSize);
        for (int i = 0; i < IntCount; i++)
        {
            for (int j = 0; j < IntSize; j++)
            {
                string Var = GetBit(i, j) ? "1" : "0";
                SB.Append(Var);
            }
        }
        return SB.ToString();
    }

    public readonly int GetFirstEmpty()
    {
        return GetFirst(0, false);
    }

    public readonly int GetFirst(int Start, bool bShouldBeSet)
    {
        int IntStart = Start / IntSize;
        int BitStart = Start - IntStart * IntSize;
        fixed (int* IntPtr = &Data0)
        {
            for (int i = IntStart; i < IntCount; i++)
            {
                int StartInInt = i == IntStart ? BitStart : 0;
                for (int j = StartInInt; j < IntSize; j++)
                {
                    var Temp = (IntPtr[i] & (1 << j)) >> j;
                    int Target = bShouldBeSet ? 1 : 0;
                    if (Temp != Target)
                        continue;

                    return i * IntSize + j;
                }
            }
        }
        return -1;
    }

    // see https://graphics.stanford.edu/~seander/bithacks.html
    public readonly int GetAmountSetBits()
    {
        int BitAmount = 0;
        fixed (int* IntPtr = &Data0)
        {
            for (int i = 0; i < IntCount; i++)
            {
                int Copy = IntPtr[i];
                Copy = Copy - ((Copy >> 1) & 0x55555555);
                Copy = (Copy & 0x33333333) + ((Copy >> 2) & 0x33333333);
                BitAmount += ((Copy + (Copy >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
            }
        }

        return BitAmount;
    }

    public readonly BitVector Clone()
    {
        BitVector Clone;
        Clone.Data0 = Data0;
        Clone.Data1 = Data1;
        Clone.Data2 = Data2;
        Clone.Data3 = Data3;
        return Clone;
    }

    public readonly override bool Equals(object obj)
    {
        if (obj is not BitVector Other)
            return false;

        return IsEqual(Other);
    }
    public readonly override int GetHashCode()
    {
        int Code = 0;
        Code += Data0.GetHashCode(); 
        Code += Data1.GetHashCode(); 
        Code += Data2.GetHashCode(); 
        Code += Data3.GetHashCode(); 
        return Code;
    }
}
