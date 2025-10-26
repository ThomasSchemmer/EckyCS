#pragma once
#include <cstdint>

namespace GAS
{
    enum class AttributeType : uint8_t
    {
        DEFAULT,
        Health,
        Mana,
        Armour
    };

    static const char * AttributeTypeStrings[] = {
        "DEFAULT",
        "Health",
        "Mana",
        "Armour"
    };    
}
