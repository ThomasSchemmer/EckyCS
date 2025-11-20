
#include "../../../../ImportsLib/src/EckyCS/Components/ComponentGroup.h"
#include "Util.h"
#include "gtest/gtest.h"
using namespace EckyCS;


TEST(Helper, ComponentGroup)
{
    auto SizeTest = sizeof(TestComponent);
    auto SizeOther = sizeof(OtherTestComponent);
    size_t SumSize0 = SumSizesTo<0, TestComponent, OtherTestComponent>();
    size_t SumSize1 = SumSizesTo<1, TestComponent, OtherTestComponent>();
    size_t SumSize2 = SumSizesTo<2, TestComponent, OtherTestComponent>();
    size_t TotalSize = TotalByteCount<TestComponent, OtherTestComponent>();
    EXPECT_EQ(SumSize0, 0);
    EXPECT_EQ(SumSize1, SizeTest);
    EXPECT_EQ(SumSize2, SizeTest + SizeOther);
    EXPECT_EQ(TotalSize, SizeTest + SizeOther);

    auto Pos1 = GetIndexOf<TestComponent, OtherTestComponent, TestComponent, OtherTestComponent>();
    auto Pos2 = GetIndexOf<OtherTestComponent, TestComponent, TestComponent, OtherTestComponent>();
    EXPECT_EQ(Pos1, 1);
    EXPECT_EQ(Pos2, 2);
}

TEST(Constructor, ComponentGroup)
{
    ComponentGroup<TestComponent, OtherTestComponent> CG(10);

    EXPECT_TRUE(CG.GetIDs() != nullptr);
    EXPECT_TRUE(CG.GetData() != nullptr);
    
    auto NameA = typeid(TestComponent).name();
    auto NameB = typeid(OtherTestComponent).name();
    auto NameC = "InvalidName";
    EXPECT_TRUE(CG.GroupID.HasFlag(NameA));
    EXPECT_TRUE(CG.GroupID.HasFlag(NameB));
    EXPECT_FALSE(CG.GroupID.HasFlag(NameC));

    
}


TEST(Get, ComponentGroup)
{
    ComponentGroup<TestComponent, OtherTestComponent> CG(10);

    auto TestSpan = CG.Get<TestComponent>();
    auto OtherSpan = CG.Get<OtherTestComponent>();
    auto Tuples = CG.GetAll<TestComponent, OtherTestComponent>();

    TestSpan[0].A = 5;
    TestSpan[1].A = 10;
    OtherSpan[0].B = 22;
    OtherSpan[1].B = 33;
    EXPECT_EQ(TestSpan[0].A, Get<TestComponent>(Tuples)[0].A);
    EXPECT_EQ(TestSpan[1].A, Get<TestComponent>(Tuples)[1].A);
    EXPECT_EQ(OtherSpan[0].B, Get<OtherTestComponent>(Tuples)[0].B);
    EXPECT_EQ(OtherSpan[1].B, Get<OtherTestComponent>(Tuples)[1].B);
}

TEST(Set, ComponentGroup)
{
    auto A = TestComponent();
    A.A = 42;
    constexpr int Target = 5;
    ComponentGroup<TestComponent, OtherTestComponent> CG(10);
    CG.SetData<TestComponent>(Target, A);
    
    auto TestSpan = CG.Get<TestComponent>();
    EXPECT_EQ(TestSpan[Target].A, A.A);
    EXPECT_NE(TestSpan[0].A, A.A);
}


TEST(ForEach, ComponentGroup)
{
    ComponentGroup<TestComponent, OtherTestComponent> CG(10);
    TestSystem TS;
    
    CG.Set(0, EntityID(), true);
    EXPECT_NE(CG.Get<TestComponent>()[0].A, TS.TargetValue);
    EXPECT_NE(CG.Get<OtherTestComponent>()[0].B, TS.TargetValue);
    CG.ForEachEntity<&TestSystem::Modify<TestComponent>, TestSystem>(TS);
    CG.ForEachEntity<&TestSystem::Test, TestSystem>(TS);
    EXPECT_EQ(CG.Get<TestComponent>()[0].A, TS.TargetValue);
    EXPECT_EQ(CG.Get<OtherTestComponent>()[0].B, TS.TargetValue);
}

TEST(ForEachWith, ComponentGroup)
{
    ComponentGroup<TestComponent, OtherTestComponent> CG(10);
    TestSystem TS;
    
    CG.Set(0, EntityID(0));
    CG.Set(1, EntityID(1));
    CG.Set(2, EntityID(2));
    EXPECT_NE(CG.Get<TestComponent>()[0].A, TS.TargetValue);
    EXPECT_NE(CG.Get<TestComponent>()[1].A, TS.TargetValue);
    EXPECT_NE(CG.Get<TestComponent>()[2].A, TS.TargetValue);

    CG.ForEachEntityWith<&TestSystem::IsOdd, &TestSystem::Modify<TestComponent>, TestSystem>(TS);
    // expect to have only touched index 1! 
    EXPECT_NE(CG.Get<TestComponent>()[0].A, TS.TargetValue);
    EXPECT_EQ(CG.Get<TestComponent>()[1].A, TS.TargetValue);
    EXPECT_NE(CG.Get<TestComponent>()[2].A, TS.TargetValue);
}

/** Tests that sets IDs and their components and tests for valid indexing */
TEST(Setter, ComponentGroup)
{
    ComponentGroup<TestComponent, OtherTestComponent> CG(10);

    constexpr int TargetIndex = 3;
    EXPECT_FALSE(CG.Has(TargetIndex));

    const auto ID = EntityID(5); 
    CG.Set(TargetIndex, ID);
    EXPECT_TRUE(CG.Has(TargetIndex));

    TestComponent Test;
    Test.A = 17;
    CG.Set(TargetIndex, ID);
    CG.SetData(TargetIndex, Test);
    EXPECT_TRUE(CG.Has(TargetIndex));
    EXPECT_EQ(CG.Get<TestComponent>()[TargetIndex].A, Test.A);

    CG.Reset(TargetIndex, true);
    EXPECT_FALSE(CG.Has(TargetIndex));
    EXPECT_NE(CG.Get<TestComponent>()[TargetIndex].A, Test.A);

    OtherTestComponent OtherTest;
    OtherTest.B = 42;
    CG.Set(TargetIndex, ID, true);
    CG.SetData(TargetIndex, Test, OtherTest);
    EXPECT_TRUE(CG.Has(TargetIndex));
    EXPECT_EQ(CG.Get<TestComponent>()[TargetIndex].A, Test.A);
    EXPECT_EQ(CG.Get<OtherTestComponent>()[TargetIndex].B, OtherTest.B);
}


// TODO: MOVE/SHRINK/SWAP