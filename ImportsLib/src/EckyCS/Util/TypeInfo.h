#pragma once
#include <vcruntime_typeinfo.h>

namespace EckyCS
{
    /**
     * Extension class to be able to hash/compare the std::type_info
     * so we can use it as an index in a map (see @ECS)
     */
    class TypeInfo
    {
        const type_info* Info;

    public:
        // automatic conversion constructor
        TypeInfo(const type_info& InInfo) : Info(&InInfo) {}
        
        bool operator<(const TypeInfo& other) const noexcept
        {
            return Info->before(*other.Info);
        }

        bool operator==(const TypeInfo& other) const noexcept
        {
            return Info == other.Info;
        }
    };
}
