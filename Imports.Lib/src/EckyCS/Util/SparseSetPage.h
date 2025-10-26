#pragma once
#include <cassert>

namespace EckyCS
{
    
    class SparseSetPage
    {
    public:
        static constexpr unsigned int PAGE_SIZE = 256;
        static constexpr int INVALID_INDEX = -1;
        int Indices[PAGE_SIZE];
        bool bIsValid = false;

        SparseSetPage() = default;

        void Fill(int Value = INVALID_INDEX)
        {
            for (unsigned int i = 0; i < PAGE_SIZE; i++)
            {
                Indices[i] = Value;
            }
        }

        void Init()
        {
            bIsValid = true;
            Fill();
        }

        bool IsEmpty() const
        {
            for (int i = 0; i < PAGE_SIZE; i++)
            {
                if (Indices[i] != INVALID_INDEX)
                    return false;
            }
            return true;
        }
    };
}
