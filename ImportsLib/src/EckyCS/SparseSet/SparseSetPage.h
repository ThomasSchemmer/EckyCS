#pragma once

namespace EckyCS
{
    
    class SparseSetPage
    {
    public:
        static constexpr unsigned int PAGE_SIZE = 256;
        static constexpr size_t INVALID_INDEX = -1;
        size_t Indices[PAGE_SIZE];
        bool bIsValid = false;

        SparseSetPage() = default;

        void Fill(size_t Value = INVALID_INDEX)
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
            for (size_t i = 0; i < PAGE_SIZE; i++)
            {
                if (Indices[i] != INVALID_INDEX)
                    return false;
            }
            return true;
        }
    };
}
