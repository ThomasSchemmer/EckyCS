#pragma once

#include <string>

namespace GAS
{
    using namespace std;
    
    /**
     * Internal representation of a gameplay tag. Uses ID to allow renaming/moving
     * Could use GUID instead of string but that lead to build errors
     */
    class GameplayTagToken
    {
    public:
        string ID;
        int Depth;
        bool bIsFolded;
        
        string CreateID()
        {
            static random_device dev;
            static mt19937 rng(dev());

            uniform_int_distribution<int> dist(0, 15);

            const char *v = "0123456789abcdef";
            constexpr bool HasDash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

            string res;
            for (const bool i : HasDash)
            {
                if (i) res += "-";
                res += v[dist(rng)];
                res += v[dist(rng)];
            }
            return res;
        }
        
        GameplayTagToken()
        {
            ID = CreateID();
        }
        GameplayTagToken(const string& Token, int Depth, bool bIsFolded)
        {
            Fill(Token, Depth, bIsFolded);
        }

        bool operator==(const GameplayTagToken& Other) const
        {
            return this->ID == Other.ID &&
                this->Depth == Other.Depth &&
                    this->bIsFolded == Other.bIsFolded;
        }
        
        bool operator==(const string& Other) const
        {
            return this->ID == Other;
        }

        void Fill(const string& nToken, int nDepth, bool nbIsFolded)
        {
            this->ID = nToken;
            this->Depth = nDepth;
            this->bIsFolded = nbIsFolded;
        }
        
        constexpr static char Divisor = '.';
    };
}
