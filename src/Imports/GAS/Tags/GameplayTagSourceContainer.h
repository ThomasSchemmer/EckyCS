#pragma once
#include <sstream>
#include <string>
#include <vector>

#include "GameplayTagToken.h"

namespace GAS{
    using namespace std;
    
    /** 
    * Container for all gameplaytags in a given global setup 
    * Only one of these should ever exist as a lookup source
    * For an obj-attached one use @GameplayTagContainer
    */
    class GameplayTagSourceContainer{
    public:
        std::vector<GameplayTagToken> Tokens;
        
        void AddTag(const std::string& NewTag)
        {
            vector<string> NewTokens = SplitString(NewTag, GameplayTagToken::Divisor);
            int FoundDepth = -1;
            int TokenIndex = 0;
            size_t FoundIndex = -1;
            for (size_t i = 0; i < Tokens.size(); ++i)
            {
                GameplayTagToken& Target = Tokens[i];

                // a previous one was mismatched
                if (Target.Depth != TokenIndex)
                    continue;

                if (NewTokens[TokenIndex] != Target.ID)
                    continue;

                TokenIndex++;
                FoundIndex = i;
                FoundDepth = Target.Depth;
            }

            if (FoundIndex == static_cast<size_t>(-1))
            {
                FoundIndex = Tokens.size() - 1;
                FoundDepth = -1;
            }

            int InsertCount = 0;
            for (size_t i = TokenIndex; i < NewTokens.size(); i++)
            {
                size_t TargetIndex = FoundIndex + 1 + InsertCount;
                int NewDepth = FoundDepth + 1 + InsertCount;
                // use stack allocation and implied copy in insert
                GameplayTagToken NewToken;
                NewToken.Fill(NewTokens[i], NewDepth, false);
                Tokens.insert(Tokens.begin() + static_cast<long long>(TargetIndex), NewToken);

                InsertCount++;
            }
        }

        bool TryGetByID(const std::string& ID, GameplayTagToken& FoundToken) const
        {
            for (auto& Token : Tokens)
            {
                if (Token.ID != ID)
                    continue;

                FoundToken = Token;
                return true;
            }
            return false;
        }

    private:
        vector<string> SplitString(const std::string& s, char delimiter) {
            vector<std::string> tokens;
            string token;
            istringstream tokenStream(s);

            while (std::getline(tokenStream, token, delimiter)) { 
                tokens.push_back(token);
            }
            return tokens;
        }
    };
}
