#pragma once
#include <map>
#include <memory>

#include "../../Util/ActionMap.h"
#include "../Effects/GameplayEffectModifier.h"

namespace GAS
{
    enum class GameplayAbilityComponentType : uint8_t;
}

namespace GAS
{
    using namespace std;
    using namespace GameImports;
    class GameplayEffectModifier;
    enum class AttributeType : unsigned char;
    
    /** 
     * Represents a runtime parameter that affects the players stats. Contained in an @AttributeSet
     * Can have modifier applied to it via @GameplayEffect
     */
    class Attribute
    {
    public:
        AttributeType Type;
        float BaseValue = 0;
        float CurrentValue = 0;

        Attribute();
        Attribute(AttributeType NewType);

        void Initialize();
        void Reset();
        void Tick();
        void AddModifier(const shared_ptr<GameplayEffectModifier>& Modifier);
        void RemoveModifier(shared_ptr<GameplayEffectModifier>& Modifier);

        ActionMap<GameplayAbilityComponentType, AttributeType> OnAttributeChanged;
        
    private:
        void ResetModifiers();
        float GetModifiedValueFor(GameplayEffectModifierType TargetType);

        map<GameplayEffectModifierType, vector<shared_ptr<GameplayEffectModifier>>> Modifiers;
    };
}
