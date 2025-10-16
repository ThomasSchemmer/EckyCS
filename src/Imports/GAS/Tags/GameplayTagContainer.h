#pragma once
#include <string>
#include <vector>

namespace GAS
{
    /** Container for all @GameplayTagToken's currently applied to a single object 
     * Contains ids instead of the usual tags
     */
    class GameplayTagContainer
    {
    public:
        std::vector<std::string> IDs;
        bool bIsEditing = false;
        bool bIsEditable = false;
        std::string Name;

        GameplayTagContainer() = default;
        GameplayTagContainer(std::string Name) : Name(std::move(Name))
        {
        }
    };
}
