#include "ISparseSet.h"

#include <cassert>

namespace EckyCS
{
    bool ISparseSet::Has(const EntityID& ID) const
    {
        size_t BaseIndex = GetBasePageIndex(ID);
        size_t MappedIndex = PageMapping.contains(BaseIndex) ?
            PageMapping.at(BaseIndex) : SparseSetPage::INVALID_INDEX;
        if (MappedIndex == SparseSetPage::INVALID_INDEX)
            return false;

        return HasInternal(ID, Pages[MappedIndex], GetIndexInPage(ID));
    }

    void ISparseSet::Remove(const EntityID& ID)
    {
        assert(Has(ID));
        size_t IndexInPage = GetIndexInPage(ID);
        size_t PageIndex = GetPageIndex(ID, false);
        if (PageIndex == SparseSetPage::INVALID_INDEX)
            return;

        auto& Page = Pages[PageIndex];
        size_t Index = Page.Indices[IndexInPage];

        auto Temp = RemoveInternal(Index);
        Page.Indices[IndexInPage] = static_cast<int>(Temp.GetID());
        Available++;
    }

    void ISparseSet::RemoveRange(const vector<EntityID>& IDs)
    {
        for (const auto& ID : IDs)
        {
            Remove(ID);
        }
    }

    size_t ISparseSet::GetCount() const
    { 
        return 0;
    }

    size_t ISparseSet::GetTotalCount() const
    {
        return 0; 
    }

    bool ISparseSet::IsFull() const
    {
        return Available == 0;
    }

    EntityID ISparseSet::RemoveInternal(size_t Index)
    {
        throw std::exception("Needs to be overwritten!");
    }

    bool ISparseSet::HasInternal(const EntityID& ID, const SparseSetPage& Page, size_t IndexInPage) const
    {
        throw std::exception("Needs to be overwritten!");
    }

    void ISparseSet::SetData(const EntityID& ID, int Offset, int Size, void* Ptr)
    {
        throw std::exception("Needs to be overwritten!");
    }

    
    void ISparseSet::SetDataBlock(int Offset, int Size, size_t StartIndex, size_t Count, void* Ptr)
    {
        throw std::exception("Needs to be overwritten!");
    }

    span<Component> ISparseSet::GetData(const EntityID& ID, size_t Offset, size_t Size)
    {
        throw std::exception("Needs to be overwritten!");
    }

    span<Component> ISparseSet::GetData(size_t Offset, size_t Size)
    {
        throw std::exception("Needs to be overwritten!");
    }

    span<EntityID> ISparseSet::GetIDSpan()
    {
        throw std::exception("Needs to be overwritten!");
    }

    size_t ISparseSet::GetTargetIndex(const EntityID& ID) const
    {
        size_t PageIndex = GetPageIndex(ID);
        if (PageIndex == SparseSetPage::INVALID_INDEX)
            return SparseSetPage::INVALID_INDEX;
        
        auto Index = GetIndexInPage(ID);
        return Pages[PageIndex].Indices[Index];
    }
    
    vector<size_t> ISparseSet::GetTargetIndices(const vector<EntityID>& IDs) const
    {
        vector<size_t> TargetIndices;
        TargetIndices.reserve(IDs.size());
        for (const EntityID& ID : IDs)
        {
            TargetIndices.push_back(GetTargetIndex(ID));
        }
        return TargetIndices;
    }
    
    unsigned int ISparseSet::GetIndexInPage(const EntityID& ID)
    {
        return ID.GetID() % SparseSetPage::PAGE_SIZE;
    }
    
    unsigned int ISparseSet::GetBasePageIndex(const EntityID& ID)
    {
        return ID.GetID() / SparseSetPage::PAGE_SIZE;
    }

    size_t ISparseSet::GetPageIndex(const EntityID& ID) const
    {
        const auto PageIndex = GetBasePageIndex(ID);
        if (!PageMapping.contains(PageIndex))
            return SparseSetPage::INVALID_INDEX;

        return PageMapping.at(PageIndex);
    }
    
    size_t ISparseSet::GetPageIndex(const EntityID& ID, bool bShouldCreate)
    {
        const auto PageIndex = GetBasePageIndex(ID);
        if (!PageMapping.contains(PageIndex))
        {
            if (bShouldCreate)
            {
                CreatePageAtMapping(PageIndex);
            }else
            {
                return SparseSetPage::INVALID_INDEX;
            }
        }
        return GetPageIndex(ID);
    }
    
    void ISparseSet::CreatePageAtMapping(size_t Index)
    {
        if (Index >= NumPages)
        {
            int asd = 5;
        }
        assert(Index >= static_cast<size_t>(0) && Index < NumPages);
        size_t FreeIndex = Pages.size();
        Pages.emplace_back();
        Pages[FreeIndex].Init();
        PageMapping[Index] = FreeIndex;
    }
}
