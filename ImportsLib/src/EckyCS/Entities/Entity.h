#pragma once
#include <tuple>

namespace EckyCS
{

    template<typename... Components>
    /**
     * Abstract definition of an Entity by listing its required components by type
     * The components will never actually be "on" the Entity, they will be instead inside
     * its @ComponentGroup, accessible *only* through the @ECS
     */
    class Entity
    {
    public:
        using RequiredComponents = std::tuple<Components...>;

        Entity() = default;
        virtual ~Entity() = default;

    protected:
        //virtual void MakeAbstract() = 0;
    };
    
    template <typename T>
    concept HasRequiredComponents = requires {
        typename T::RequiredComponents;
    };
    

}
