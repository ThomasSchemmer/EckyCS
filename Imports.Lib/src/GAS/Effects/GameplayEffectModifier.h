#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace GAS
{
    using namespace std;
    enum class AttributeType : unsigned char;
    class Attribute;
    class GameplayAbilityComponent;
    
    enum class GameplayEffectModifierType : uint8_t
    {
        Add,
        Multiply,
        Override
    };
    
    /**
     * Baseclass that describes how an effect would modify 
     * an attribute, eg by increasing it
     */
    class GameplayEffectModifier : enable_shared_from_this<GameplayEffectModifier>
    {
    public:
        AttributeType AttributeType;
        shared_ptr<Attribute> Attribute;
        GameplayEffectModifierType Operation;
        /** How often should this be applied? 0 = only once*/
        float PeriodS = 0;
        /** By how much should it be applied? */
        float Value = 0;

        void SetTarget(const shared_ptr<GameplayAbilityComponent>& NewTarget);
        void Execute();
        void Revert() const;
        void Tick(float Delta);
        string GetDescription(int Multiplier = 1) const;
    private:
        float TimeSinceLastActivatedS = -1;
        shared_ptr<GameplayAbilityComponent> Target;

        shared_ptr<GameplayEffectModifier> Invert() const;
        string GetAddDescription(int Multiplier = 1) const;
        string GetNormalDescription() const;
        string GetAttributeDescription() const;
        string GetOperationDescription() const;
        string GetOperationPrepositionDescription() const;
    };
}
