#pragma once
#include <random>
#include <unordered_set>

#include "GameplayTags.h"

namespace GAS
{    
    class GameplayTagMask
    {
    public:
        void Set(const string& ID)
        {
            if (Mask.contains(ID))
                return;
            Mask.insert(ID);
        }

        void Remove(const string& ID)
        {
            if (!Mask.contains(ID))
                return;
            Mask.erase(ID);
        }

        void Set(const std::vector<string>& IDs)
        {
            for (auto& ID : IDs)
            {
                Set(ID);
            }
        }

        bool HasID(const string& ID, bool bAllowPartial = false)
        {
            if (Mask.contains(ID))
                return true;

            if (!bAllowPartial)
                return false;

            string Parent;
            if (!GameplayTags::Get().TryGetParentID(ID, Parent))
                return false;

            return HasID(Parent, bAllowPartial);
        }

        void Combine(const GameplayTagMask& Other)
        {
            Mask.insert(Other.Mask.begin(), Other.Mask.end());
        }
        
    private:
        std::unordered_set<string> Mask;
    };
    
}