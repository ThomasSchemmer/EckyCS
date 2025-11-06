#pragma once
#include <cstdint>

namespace EckyCS
{
    
    /**
     * Describes a system of the ECS:
     * modifies components of entities according to certain rules
     */
    class System
    {
    public:
        virtual void StartSystem() {}
        virtual void Tick(float Delta){}
        virtual void FixedTick(float Delta){}
        virtual void LateTick(float Delta){}
        virtual void OnDrawGizmos() {}

        System() = default;
        virtual ~System() = default;
    };
}
