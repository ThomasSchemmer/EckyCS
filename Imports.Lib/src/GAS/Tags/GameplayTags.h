#pragma once
#include "GameplayTagSourceContainer.h"

namespace GAS {
    
    /** 
     * Main access point for all gameplay tag related things
     * Should only be created once per project
     * If you want to add / query tags, do it here
     */
    class GameplayTags{
    public:
        GameplayTagSourceContainer Container;

        void AddTag(string Tag)
        {
            Container.AddTag(Tag);
        }

        int GetParentIndex(size_t Index) const
        {
            GameplayTagToken Child = Container.Tokens[Index];
            
            // parents have to come beforehand, so we can go backwards
            for (int i = static_cast<int>(Index); i >= 0; i--)
            {
                GameplayTagToken PotentialParent = Container.Tokens[i];
                if (PotentialParent.Depth == Child.Depth - 1)
                    return i;
            }
            return -1;
        }

        [[nodiscard]]
        int GetSelfIndex(const string& ID) const
        {
            for (int i = 0; i < static_cast<int>(Container.Tokens.size()); i++)
            {
                GameplayTagToken PotentialSelf = Container.Tokens[i];
                if (PotentialSelf.ID == ID)
                    return i;
            }
            return -1;
        }

        bool TryGetParentID(const string& ID, string& ParentID) const
        {
            size_t SelfIndex = GetSelfIndex(ID);
            if (SelfIndex == static_cast<size_t>(-1))
                return false;

            int ParentIndex = GetParentIndex(SelfIndex);
            if (ParentIndex == -1)
                return false;

            ParentID = Container.Tokens[ParentIndex].ID;
            return true;
        }

        static GameplayTags& Get()
        {
            static GameplayTags Instance;
            if (Instance.Container.Tokens.empty())
            {
                // todo: load
            }
            return Instance;
        }
    };
}
