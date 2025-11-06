#pragma once
#include "../../../../Imports.Lib/src/EckyCS/Components/ComponentGroup.h"
#include "../../../../Imports.Lib/src/EckyCS/Entities/Entity.h"
#include "../../../../Imports.Lib/src/EckyCS/Systems/System.h"

using namespace std;
using namespace EckyCS;

class TestComponent: public Component
{
public:
    int A = 0;
};

class OtherTestComponent: public Component
{
public:
    int B = 0;
};


class TestSystem : public System
{
public:
    int TargetValue = 5;
    
    template <typename... TargetTypes>
    bool Modify(ComponentGroupIdentifier GroupID, size_t Index, View<TargetTypes...>& View)
    {
        Get<TestComponent>(View)[Index].A = TargetValue;
        return true;
    }

    bool Test(ComponentGroupIdentifier GroupID, size_t Index, View<TestComponent, OtherTestComponent>& View)
    {
        Get<OtherTestComponent>(View)[Index].B = TargetValue;
        return true;
    }
    
    bool IsOdd(ComponentGroupIdentifier GroupID, size_t Index, View<TestComponent>& View)
    {
        return Index % 2 == 1; 
    }
};

class TestEntity : public Entity<TestComponent, OtherTestComponent>
{
    
};