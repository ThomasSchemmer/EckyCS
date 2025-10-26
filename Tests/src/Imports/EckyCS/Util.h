#pragma once
#include "../../../../Imports.Lib/src/EckyCS/Components/ComponentGroup.h"

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


class TestSystem
{
public:
    int TargetValue = 5;
    
    template <typename... TargetTypes>
    void Modify(ComponentGroupIdentifier GroupID, EntityID ID, View<TargetTypes...>& View)
    {
        get<0>(View)[0].A = TargetValue;
    }

    void Test(ComponentGroupIdentifier GroupID, EntityID ID, View<TestComponent, OtherTestComponent>& View)
    {
        get<1>(View)[0].B = TargetValue;
    }
};