#pragma once
#include <cstdint>

namespace EckyCS
{
    class ECS;
    
    /**
     * Describes a system of the ECS:
     * modifies components of entitises according to certain rules
     */
    class System
    {
    public:
        virtual void StartSystem(shared_ptr<ECS>& Ecs) {}
        virtual void Tick(float Delta){}
        virtual void FixedTick(float Delta){}
        virtual void LateTick(float Delta){}
        virtual void OnDrawGizmos() {}

        System() = default;
        virtual ~System() = default;
    };
}
