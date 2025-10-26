#include "BitVector.h"

#include <assert.h>
#include <cstddef>
#include <memory>

#include "ComponentAllocator.h"

namespace EckyCS
{
    using namespace std;

    // see https://graphics.stanford.edu/~seander/bithacks.html
    int BitVector::GetAmountSetBits() const
    {
        auto Ptr = reinterpret_cast<const int*>(Data);
        int BitAmount = 0;
        for (int i = 0; i < IntCount; i++)
        {
            // make an actual copy to respect "const"
            auto Copy = *(Ptr + i);
            Copy = Copy - ((Copy >> 1) & 0x55555555);
            Copy = (Copy & 0x33333333) + ((Copy >> 2) & 0x33333333);
            BitAmount += ((Copy + (Copy >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
        }

        return BitAmount;
    }

    BitVector BitVector::And(const BitVector& Other) const
    {
        BitVector Result;
        for (int i = 0; i < ByteCount; i++)
        {
            Result.Data[i] = Data[i] & Other.Data[i];
        }
        return Result;
    }

    BitVector BitVector::Or(const BitVector& Other) const
    {
        BitVector Result;
        for (int i = 0; i < ByteCount; i++)
        {
            Result.Data[i] = Data[i] | Other.Data[i];
        }
        return Result;
    }

    BitVector BitVector::Subtract(const BitVector& Other) const
    {
        BitVector Result;
        for (int i = 0; i < ByteCount; i++)
        {
            int Temp = static_cast<int>(Data[i]) - static_cast<int>(Other.Data[i]);
            Result.Data[i] = static_cast<byte>(std::max(Temp, 0));
        }
        return Result;
    }

    void BitVector::Set(int Pos, bool Value)
    {
        assert(Pos >= 0 && Pos < TotalLength, "Out of bounds");
        int BytePos, PosInByte;
        ToBitPos(Pos, BytePos, PosInByte);
        if (Value)
        {
            SetBit(BytePos, PosInByte);
        }else
        {
            ClearBit(BytePos, PosInByte);
        }
    }

    bool BitVector::Get(int Pos) const
    {
        int BytePos, PosInByte;
        ToBitPos(Pos, BytePos, PosInByte);
        return GetBit(BytePos, PosInByte);
    }

    bool BitVector::operator==(const BitVector& Other) const
    {
        for (int i = 0; i < ByteCount; i++)
        {
            if (Data[i] != Other.Data[i])
                return false;
        }
        return true;
    }

    bool BitVector::operator!=(const BitVector& Other) const
    {
        return !(*this == Other);
    }

    int BitVector::GetSelfIndexOf(int Target) const
    {
        if (!Get(Target))
            return -1;

        int Count = 0;
        for (int i = 0; i < Target; ++i)
        {
            Count += Get(i);
        }
        return Count;
    }

    vector<int> BitVector::GetSelfIndicesOf(const vector<int>& Targets) const
    {
        vector<int> Result;
        Result.reserve(Targets.size());
        for (auto&& Target : Targets)
        {
            Result.emplace_back(GetSelfIndexOf(Target));
        }
        return Result;
    }

    int BitVector::GetFirstIndexWith(int Start, const bool bShouldBeSet) const
    {
        int ByteStart = Start / ByteSize;
        int BitStart = Start - ByteStart * ByteSize;

        // since we don't start at index 0, we have to skip some indices!
        for (int ByteIndex = ByteStart; ByteIndex < ByteCount; ++ByteIndex)
        {
            int StartInByte = ByteIndex == ByteStart ? BitStart : 0;
            for (int BitIndex = StartInByte; BitIndex < BitsPerByte; ++BitIndex)
            {
                // take only the important bit into consideration
                auto Temp = (0x1 & static_cast<int>(Data[ByteIndex] >> BitIndex)) > 0;
                if (Temp != bShouldBeSet)
                    continue;

                return ByteIndex * ByteSize + BitIndex;
            }
        }
        return -1;
    }

    vector<int> BitVector::GetAllIndicesWith(const bool bShouldBeSet) const
    {
        int Count = GetAmountSetBits();
        Count = bShouldBeSet ? Count : TotalLength - Count;
        vector<int> Result;
        Result.reserve(Count);

        for (int i = 0; i < TotalLength; ++i)
        {
            auto Temp = Get(i);
            if (Temp != bShouldBeSet)
                continue;

            Result.emplace_back(i);
        }
        return Result;
    }

    void BitVector::SetBit(int BytePos, int PosInByte)
    {
        Data[BytePos] |= static_cast<byte>(1 << PosInByte);
    }

    bool BitVector::GetBit(int BytePos, int PosInByte) const
    {
        return static_cast<int>(Data[BytePos] & static_cast<byte>(1 << PosInByte)) > 0;
    }

    void BitVector::ClearBit(int BytePos, int PosInByte)
    {
        Data[BytePos] &= static_cast<byte>(0 << PosInByte);
    }

    void BitVector::ToBitPos(int Pos, int& OutPosByte, int& OutPosBit)
    {
        OutPosByte = Pos / ByteSize;
        OutPosBit = Pos - OutPosByte * ByteSize;
    }
}
