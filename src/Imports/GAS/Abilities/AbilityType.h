#pragma once
#include <cstdint>

namespace GAS
{
    enum class AbilityType : uint8_t
    {
        DEFAULT,
        Fireball,
        Shovel,
        Harvest,
        Interact,
        OpenInventory,
    };
}
