
#include "../../../../Imports.Lib/src/EckyCS/Util/BitVector.h"

#include "gtest/gtest.h"

TEST(SetBit, BitVector)
{
    EckyCS::BitVector V;
    constexpr int Target = 15;
    EXPECT_FALSE(V.Get(Target));
    V.Set(Target, true);
    EXPECT_TRUE(V.Get(Target));
}

TEST(ToBitPos, BitVector)
{
    constexpr int Target = 27;
    int PosByte, PosInByte;
    EckyCS::BitVector::ToBitPos(Target, PosByte, PosInByte);
    EXPECT_EQ(PosByte, 3);
    EXPECT_EQ(PosInByte, 3);
}

TEST(AmountSetBits, BitVector)
{
    EckyCS::BitVector V;
    V.Set(0, true);
    V.Set(4, true);
    V.Set(8, true);
    const int Result = V.GetAmountSetBits();
    EXPECT_EQ(Result, 3);
}


TEST(Or, BitVector)
{
    EckyCS::BitVector V1;
    EckyCS::BitVector V2;
    V1.Set(0, true);
    V1.Set(4, true);
    V1.Set(8, true);
    auto&& V3 = V2.Or(V1);
    EXPECT_EQ(V1.GetAmountSetBits(), 3);
    EXPECT_EQ(V2.GetAmountSetBits(), 0);
    EXPECT_EQ(V3.GetAmountSetBits(), 3);
}

TEST(And, BitVector)
{
    EckyCS::BitVector V1;
    EckyCS::BitVector V2;
    V1.Set(0, true);
    V1.Set(4, true);
    V1.Set(8, true);

    V2.Set(4, true);
    // intentionally duplicated
    V2.Set(4, true);
    V2.Set(8, true);
    
    auto&& V3 = V2.And(V1);
    EXPECT_EQ(V1.GetAmountSetBits(), 3);
    EXPECT_EQ(V2.GetAmountSetBits(), 2);
    EXPECT_EQ(V3.GetAmountSetBits(), 2);
}

TEST(Equals, BitVector)
{
    EckyCS::BitVector V1;
    EckyCS::BitVector V2;
    EckyCS::BitVector V3;
    V1.Set(0, true);
    V1.Set(4, true);
    V1.Set(8, true);

    V2.Set(4, true);
    V2.Set(8, true);
    
    V3.Set(0, true);
    V3.Set(8, true);
    
    EXPECT_TRUE(V1 != V2);
    EXPECT_TRUE(V1 != V3);
    EXPECT_TRUE(V2 != V3);
    
    V3.Set(0, false);
    V3.Set(4, true);
    EXPECT_TRUE(V2 == V3);
}

TEST(SelfIndex, BitVector)
{
    EckyCS::BitVector V;
    constexpr int PosA = 5;
    constexpr int PosB = 3;

    // fake registry, since we are working on agnostic bitvectors
    // and don't have actual components
    EXPECT_TRUE(V.GetSelfIndexOf(PosA) == -1);
    EXPECT_TRUE(V.GetSelfIndexOf(PosB) == -1);
    
    V.Set(PosA, true);
    V.Set(PosB, true);
    EXPECT_TRUE(V.GetSelfIndexOf(PosA) == 1);
    EXPECT_TRUE(V.GetSelfIndexOf(PosB) == 0);
}

TEST(FirstIndex, BitVector)
{
    EckyCS::BitVector V;
    constexpr int PosA = 5;
    constexpr int PosB = 3;
    EXPECT_TRUE(V.GetFirstIndexWith(PosB, true) == -1);
    EXPECT_TRUE(V.GetFirstIndexWith(PosA, false) == PosA);
    
    V.Set(PosA, true);
    V.Set(PosB, true);
    EXPECT_TRUE(V.GetFirstIndexWith(PosB, true) == PosB);
    EXPECT_TRUE(V.GetFirstIndexWith(PosB + 1, true) == PosA);
    EXPECT_TRUE(V.GetFirstIndexWith(PosA, false) == PosA + 1);
}

TEST(ContainedIndices, BitVector)
{
    EckyCS::BitVector V;
    constexpr int PosA = 5;
    constexpr int PosB = 3;
    EXPECT_TRUE(V.GetAllIndicesWith(true).empty());
    EXPECT_FALSE(V.GetAllIndicesWith(false).empty());
    
    V.Set(PosA, true);
    V.Set(PosB, true);
    auto&& List = V.GetAllIndicesWith(true);
    EXPECT_TRUE(List.size() == 2);
    EXPECT_TRUE(std::find(List.begin(), List.end(), PosA) != List.end());
    EXPECT_TRUE(std::find(List.begin(), List.end(), PosB) != List.end());
}


