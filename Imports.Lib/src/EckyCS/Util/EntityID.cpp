#include "EntityID.h"

namespace EckyCS
{
    
    unsigned int EntityID::ID_OFFSET = 8;
    unsigned int EntityID::ID_MASK = 0xFFFFFF00;
    unsigned int EntityID::VERSION_MASK = 0x000000FF;
    unsigned int EntityID::INVALID = static_cast<unsigned int>(pow(2, 8)) - 1;
}
