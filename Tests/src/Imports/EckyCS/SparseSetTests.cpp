#include "Util.h"
#include "../../../../Imports.Lib/src/EckyCS/Util/SparseSet.h"

#include "gtest/gtest.h"
using namespace std;
using namespace EckyCS;


TEST(Constructor, SparseSet)
{
    auto Set = SparseSet<TestComponent>(10, 10);
    EXPECT_TRUE(Set.IsValid());
}


TEST(Add, SparseSet)
{
    auto Set = SparseSet<TestComponent>(10, 10);
    // random nrs to force multiple pages
    auto ID0 = EntityID(15);
    auto ID1 = EntityID(14);
    auto ID2 = EntityID(170);
    auto ID3 = EntityID(278);
    auto ID4 = EntityID(900);
    
    Set.Add(ID2);
    Set.Add(ID1);
    Set.Add(ID0);
    EXPECT_TRUE(Set.Has(ID0));
    EXPECT_TRUE(Set.Has(ID1));
    EXPECT_TRUE(Set.Has(ID2));
    EXPECT_EQ(Set.GetTargetIndex(ID0), 2);
    EXPECT_EQ(Set.GetTargetIndex(ID1), 1);
    EXPECT_EQ(Set.GetTargetIndex(ID2), 0);

    Set.Remove(ID1);
    EXPECT_NE(Set.GetTargetIndex(ID1), 1);
    Set.Add(ID3);
    EXPECT_EQ(Set.GetTargetIndex(ID3), 1);
    Set.Add(ID4);
    EXPECT_EQ(Set.GetTargetIndex(ID4), 3);
}


TEST(AddData, SparseSet)
{
    SparseSet<TestComponent, OtherTestComponent> Set(10, 150);
    auto ID = EntityID(15);
    auto Test = TestComponent();
    Test.A = 42;
    
    auto View = Set.GetData(ID);
    EXPECT_EQ(Get<TestComponent>(View).size(), 0);

    Set.Add(ID, Test);
    // View is still empty, need to re-query but that's fine
    EXPECT_EQ(Get<TestComponent>(View).size(), 0);
    
    auto View1 = Set.GetData(ID);
    EXPECT_EQ(Get<TestComponent>(View1).size(), 1);
    EXPECT_EQ(Get<TestComponent>(View1)[0].A, Test.A);
    
    Set.RemoveRange({ID});
    auto View2 = Set.GetData(ID);
    EXPECT_EQ(Get<TestComponent>(View2).size(), 0);
    EXPECT_EQ(Set.GetCount(), 0);
}

TEST(ForEach, SparseSet)
{
    SparseSet<TestComponent, OtherTestComponent> Set(10, 150);
    TestSystem TS;
    auto ID = EntityID(15);
    auto Test = TestComponent();
    Test.A = 42;

    // TODO: make botch a ForEach with GroupID, EntityID, View and Index
    // as well as a ForEachGroup with GroupID, View and Count
    Set.Add(ID, Test);
    //EXPECT_NE([0].A, TS.TargetValue);
    //EXPECT_NE(Set.Get<OtherTestComponent>()[0].B, TS.TargetValue);
    Set.ForEach<&TestSystem::Modify<TestComponent>, TestSystem>(TS);
    Set.ForEach<&TestSystem::Test, TestSystem>(TS);
    //EXPECT_EQ(Set.Get<TestComponent>()[0].A, TS.TargetValue);
    //EXPECT_EQ(Set.Get<OtherTestComponent>()[0].B, TS.TargetValue);
}


// overwrite aka add twice and cheeck if data is updated

// TODO: Get/Set/Move/Shrink, Delete and add! / swap