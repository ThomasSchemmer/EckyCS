
#include "../../../Imports.Lib/src/EckyCS/Util/BitVector.h"
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
