#pragma once
#include <cstdint>
#include <memory>

#include "../GameplayAbilityComponent.h"

namespace GAS
{
    enum class InstancingPolicy : uint8_t
    {
        InstancedPerExecution,
        InstancedPerActor,
        NonInstanced
    };

    /**
     * Standardised format for getting an instantiated version of an object
     */
    class GameplayAbilityInstanceable : enable_shared_from_this<GameplayAbilityInstanceable>
    {
    protected:
        virtual bool IsSelfOnActor(const shared_ptr<GameplayAbilityComponent>& Actor) = 0;
        
		template<class T>
        requires std::is_base_of_v<GameplayAbilityInstanceable, T>
        std::shared_ptr<T> GetByInstancingActor(const shared_ptr<GameplayAbilityComponent>& Actor)
        {
		    if (IsSelfOnActor(Actor))
		    {
		        return dynamic_pointer_cast<T>(shared_from_this());
		    }
            
            return std::make_shared<T>();
        }
        
    public:
        InstancingPolicy Policy;
        
        virtual ~GameplayAbilityInstanceable() = default;

        /** Either creates a shallow copy, new version or itself depending on policy */
		template<class T>
        requires std::is_base_of_v<GameplayAbilityInstanceable, T>
        std::shared_ptr<T> GetByInstancing(const shared_ptr<GameplayAbilityComponent>& Actor)
        {
            switch (Policy)
            {
            case InstancingPolicy::InstancedPerExecution:
                    return std::make_shared<T>();
            case InstancingPolicy::InstancedPerActor:
                return GetByInstancingActor<T>(Actor);
            case InstancingPolicy::NonInstanced:
                return dynamic_pointer_cast<T>(shared_from_this());
            }
            return nullptr;
        }
        
    };
}
