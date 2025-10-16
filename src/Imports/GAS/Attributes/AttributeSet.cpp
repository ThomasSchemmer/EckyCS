#include "AttributeSet.h"

#include "Attribute.h"

namespace GAS
{
    Attribute& AttributeSet::operator[](AttributeType Type)
    {
        if (Attributes.contains(Type))
        {
            Attributes.emplace(Type, new Attribute());
        }
        return *Attributes[Type];
    }

    void AttributeSet::Add(AttributeType Type, const shared_ptr<Attribute>& Attribute)
    {
        if (!Attributes.contains(Type))
        {
            Attributes.emplace(Type, Attribute);
        }
        Attributes[Type] = Attribute;
    }

    void AttributeSet::Initialize() const
    {
        for (auto& Pair : Attributes)
        {
            Pair.second->Initialize();
        }
    }

    void AttributeSet::Tick()
    {
        for (auto& Pair : Attributes)
        {
            auto& Attribute = Pair.second;
            const float OldValue = Attribute->CurrentValue;
            Attribute->Tick();
            const float NewValue = Attribute->CurrentValue;
            if (abs(OldValue - NewValue) > 0.001f)
            {
                OnAnyAttributeChanged.ForEach(Attribute);
            }
        }
    }


    void AttributeSet::Reset()
    {
        for (auto& Pair : Attributes)
        {
            Pair.second->Reset();
            OnAnyAttributeChanged.ForEach(Pair.second);
        }
    }

}
