#include "Util.h"
#include "../../../../ImportsLib/src/EckyCS/ECS.h"
#include "../../../../ImportsLib/src/EckyCS/Entities/EntityGenerator.h"
#include "gtest/gtest.h"
using namespace std;

TEST(Constructor, EckyCS)
{
    ECS ECS;
    ECS.StartService();
    ECS.AddSystem(make_shared<TestSystem>());
}

TEST(RegisterEntity, EckyCS)
{
    auto Ecs = make_shared<ECS>();
    Ecs->StartService();
    
    EntityID ID;
    TestComponent TestA;
    TestA.A = 42;
    ComponentGroupIdentifier GroupID;
    EntityGenerator::Ecs = Ecs;
    EntityGenerator::TryCreate<TestEntity>(GroupID, ID, TestA);

    auto TargetSet = Ecs->GetOrCreateSet(GroupID);
    EXPECT_TRUE(TargetSet->Has(ID));
    auto View = TargetSet->GetData<TestComponent>(ID);
    EXPECT_EQ(Get<TestComponent>(View)[0].A, TestA.A);

    OtherTestComponent TestB;
    TestB.B = 69;
    TargetSet->SetData<OtherTestComponent>(ID, TestB);
    auto View2 = TargetSet->GetData<OtherTestComponent>(ID);
    EXPECT_EQ(Get<OtherTestComponent>(View2)[0].B, TestB.B);

    TestA.A = -123;
    TargetSet->SetData(ID, TestA);
    // since views are basically Ptrs, it should auto update - no need to recreate!
    EXPECT_EQ(Get<TestComponent>(View)[0].A, TestA.A);
}


TEST(MultipleIDs, EckyCS)
{
    auto Ecs = make_shared<ECS>();
    Ecs->StartService();
    
    EntityID ID0, ID1, ID2, ID3;
    ComponentGroupIdentifier GroupID0, GroupID1, GroupID2, GroupID3;
    EntityGenerator::Ecs = Ecs;
    EntityGenerator::TryCreate<TestEntity>(OUT GroupID0, OUT ID0);
    EntityGenerator::TryCreate<TestEntity>(OUT GroupID1, OUT ID1);
    EntityGenerator::TryCreate<TestEntity>(OUT GroupID2, OUT ID2);

    EXPECT_EQ(GroupID0, GroupID1, GroupID2);
    auto Set = Ecs->GetOrCreateSet(GroupID0);
    EXPECT_TRUE(Set->Has(ID0));
    EXPECT_TRUE(Set->Has(ID1));
    EXPECT_TRUE(Set->Has(ID2));

    Ecs->DeleteEntity(ID1);
    EntityGenerator::TryCreate<TestEntity>(OUT GroupID3, OUT ID3);
    
    EXPECT_EQ(GroupID0, GroupID3);
    EXPECT_TRUE(Set->Has(ID0));
    EXPECT_FALSE(Set->Has(ID1));
    EXPECT_TRUE(Set->Has(ID2));
    EXPECT_TRUE(Set->Has(ID3));
}