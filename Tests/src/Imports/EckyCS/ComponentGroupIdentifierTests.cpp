#include <memory.h>
#include "gtest/gtest.h"
#include "../../../../ImportsLib/src/EckyCS/Components/ComponentGroupIdentifier.h"
#include "../../../../ImportsLib/src/EckyCS/Components/Component.h"
#include "../../../../ImportsLib/src/EckyCS/Util/ComponentAllocator.h"

class TestComponent : public EckyCS::Component
{
    // just a stub to allow registering
};

TEST(Flags, ComponentGroupID)
{
    auto Ptr = std::make_shared<TestComponent>();
    auto&& Name = EckyCS::ComponentAllocator::Register(Ptr);
    EckyCS::ComponentGroupIdentifier CGI;
    EXPECT_FALSE(CGI.HasFlag(Name));
    
    CGI.AddFlag(Name);
    EXPECT_TRUE(CGI.HasFlag(Name));
    EXPECT_TRUE(CGI.HasAllFlags({Name}));
    EXPECT_TRUE(CGI.HasAnyFlag({Name}));
    EXPECT_TRUE(CGI.GetAmountOfFlags() == 1);

    CGI.RemoveFlag(Name);
    EXPECT_FALSE(CGI.HasFlag(Name));
    
}

TEST(Indices, ComponentGroupID)
{
    auto Ptr = std::make_shared<TestComponent>();
    auto&& Name = EckyCS::ComponentAllocator::Register(Ptr);
    EckyCS::ComponentGroupIdentifier CGI;
    EXPECT_EQ(CGI.GetSelfIndexOf(Name), -1);
    
    CGI.AddFlag(Name);
    EXPECT_EQ(CGI.GetSelfIndexOf(Name), 0);

    auto&& List = CGI.GetContainedTypes();
    EXPECT_EQ(List.size(), 1);
    EXPECT_EQ(List[0], Name);
}

