#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace EckyCS
{
    using namespace std;
    
    /** 
     * "string" of bits indicating whether something is represented by this vector or not
     * Length of 128
     */
    class BitVector
    {
    public:

        BitVector(): Data{} {}
        
        BitVector And(const BitVector& Other) const;
        BitVector Or(const BitVector& Other) const;
        BitVector Subtract(const BitVector& Other) const;
        
        void Set(int Pos, bool Value);
        bool Get(int Pos) const;
        int GetAmountSetBits() const;
        /** Returns the internal offset of the type index inside the flags */
        int GetSelfIndexOf(int Target) const;
        vector<int> GetSelfIndicesOf(const vector<int>& Targets) const;
        /** Returns the first index after start that's set or not */
        int GetFirstIndexWith(int Start, bool bShouldBeSet) const;
        vector<int> GetAllIndicesWith(bool bShouldBeSet) const;
        
        bool operator==(const BitVector& Other) const;
        bool operator!=(const BitVector& Other) const;

        /** Splits a global index into a byte index and an index inside the byte */
        static void ToBitPos(int Pos, int& OutPosByte, int& OutPosBit);
        
    protected:
        void SetBit(int BytePos, int PosInByte);
        bool GetBit(int BytePos, int PosInByte) const;
        void ClearBit(int BytePos, int PosInByte);
                
        static constexpr int TotalLength = 128;
        static constexpr int BitsPerByte = 8;
        static constexpr int ByteSize = sizeof(std::byte) * BitsPerByte;  
        static constexpr int IntSize = sizeof(int) * BitsPerByte;  
        static constexpr int ByteCount = TotalLength / ByteSize;
        static constexpr int IntCount = TotalLength / IntSize;

        std::byte Data[ByteCount];
    };
}
