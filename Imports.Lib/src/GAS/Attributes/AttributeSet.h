#pragma once
#include <map>
#include <memory>

#include "../../Util/ActionMap.h"

namespace GAS
{
    enum class GameplayAbilityComponentType : uint8_t;
    using namespace std;
    using namespace GameImports;
    class Attribute;
    enum class AttributeType : unsigned char;
    
    /**
     * Container for multiple @Attribute which together form the
     * backbone of player stats
     */
    class AttributeSet
    {
    public:
        map<AttributeType, shared_ptr<Attribute>> Attributes;

        ~AttributeSet() = default;
        void Initialize() const;
        void Tick();

        // allows for lazy init
        Attribute& operator [](AttributeType Type);
        void Add(AttributeType Type, const shared_ptr<Attribute>& Attribute);
        void Reset();

        static AttributeSet& Get()
        {
            static AttributeSet Instance;
            if (Instance.Attributes.empty())
            {
                // load instance
            }
            return Instance;
        }

        ActionMap<GameplayAbilityComponentType, shared_ptr<Attribute>> OnAnyAttributeChanged;
    };
}
