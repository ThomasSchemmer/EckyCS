#pragma once
#include "EckyCSHeader.h"
#include "SparseSetPage.h"
#include "../Components/ComponentGroup.h"

namespace EckyCS
{
    class ComponentGroupIdentifier;
}

namespace EckyCS
{
    /** 
     * Main structure to hold efficient data for entities
     * Has two containers: 
     *  - a sparse (paginated) mapping, where the @EntityID is the key
     *  - a dense class containing different @IComponent arrays with the actual values
     *  
     *  since we also support "empty" Components every function must be able to return / handle nulls!
     */
    template <typename ... Types>
    requires AllComponents<Types...>
    class SparseSet
    {
    public:
        SparseSet(int NumPages, int ExpectedEntities) :
            Components(ExpectedEntities), NumPages(NumPages)
        {
            GroupID.AddFlags<Types...>();
            Pages.reserve(NumPages);
            SetupEmptyRedirectors(0, ExpectedEntities);
        }
        
        ~SparseSet() = default;

        int GetTargetIndex(const EntityID& ID) const
        {
            int PageIndex = GetPageIndex(ID);
            if (PageIndex == SparseSetPage::INVALID_INDEX)
                return SparseSetPage::INVALID_INDEX;
            
            auto Index = GetIndexInPage(ID);
            return Pages[PageIndex].Indices[Index];
        }
        
        int GetCount() 
        {
            return Components.Count - Available;
        }
        
        bool Has(const EntityID& ID) const
        {
            int BaseIndex = GetBasePageIndex(ID);
            int MappedIndex = PageMapping.contains(BaseIndex) ?
                PageMapping.at(BaseIndex) : SparseSetPage::INVALID_INDEX;
            if (MappedIndex == SparseSetPage::INVALID_INDEX)
                return false;

            return Has(ID, Pages[MappedIndex], GetIndexInPage(ID));
        }
        
        bool IsFull() const
        {
            return Available == 0;
        }

        template <typename ... SubTypes>
        requires AllContainedIn<tuple<SubTypes...>, tuple<Types...>>
        void Add(const EntityID& ID, SubTypes... Data)
        {
            int PageIndex = GetPageIndex(ID, true);
            if (PageIndex == SparseSetPage::INVALID_INDEX)
                return;
            
            int Index = GetIndexInPage(ID);
            auto& Page = Pages[PageIndex];
            if (Has(ID, Page, Index))
            {
                Components.Set(Page.Indices[Index], ID);
                return;
            }

            if (IsFull())
            {
                MoveData();
            }

            Page.Indices[Index] = GetFreeValueIndex();
            Components.Set(Page.Indices[Index], ID, true);
            Components.SetData(Page.Indices[Index], Data...);
        }
        
        View<Types...> GetData(const EntityID& ID) 
        {
            if (!Has(ID))
                return View<Types...>();

            int TargetIndex = GetTargetIndex(ID);
            return Components.Get(TargetIndex);
        }
        
        void Remove(const EntityID& ID)
        {
            assert(Has(ID));
            int IndexInPage = GetIndexInPage(ID);
            int PageIndex = GetPageIndex(ID, false);
            if (PageIndex == SparseSetPage::INVALID_INDEX)
                return;

            auto& Page = Pages[PageIndex];
            int Index = Page.Indices[IndexInPage];

            // actually want a copy here
            EntityID Temp = NextAvailableComponent;
            NextAvailableComponent = EntityID(Index, EntityID::INVALID);
            Components.Reset(Index);
            Components.Set(Index, Temp, false); //already reset
            Page.Indices[IndexInPage] = static_cast<int>(Temp.GetID());
            Available++;
        }
        
        void RemoveRange(const vector<EntityID>& IDs)
        {
            for (auto& ID : IDs)
            {
                Remove(ID);
            }
        }
        
        //void Swap(const EntityID& ID1, const EntityID& ID2)
        //{
        // TODO: swap
        //    int PageIndexA = GetPageIndex(ID1, false);
        //    int PageIndexB = GetPageIndex(ID2, false);
        //    auto& PageA = Pages[PageIndexA];
        //    auto& PageB = Pages[PageIndexB];
        //    int IndexInPageA = GetIndexInPage(ID1);
        //    int IndexInPageB = GetIndexInPage(ID2);
        //    int DenseIndexA = PageA.Indices[IndexInPageA];
        //    int DenseIndexB = PageB.Indices[IndexInPageB];
        //    Values?.Swap(DenseIndexA, DenseIndexB);
        //    PageA.Indices[IndexInPageA] = DenseIndexB;
        //    PageB.Indices[IndexInPageB] = DenseIndexA;
        //}
        
        bool IsValid() const
        {
            return Components.IsValid();
        }
        
        template <auto Method, typename System>
        void ForEach(System& system)
        {
            Components.template ForEach<Method>(system);
        }
        //template <auto Method, typename System>
        //void CheckEntity(System& system);
        //template <auto Method, typename System>
        //EntityID SelectEntityFrom(const vector<EntityID>& IDs, System& system);
        //template <auto Method, typename System>
        //void ForEachEntityFrom(const vector<EntityID>& IDs, System& system);

    protected:
        ComponentGroup<Types...> Components;
        vector<SparseSetPage> Pages;
        map<int, int> PageMapping;
        int NumPages;
        ComponentGroupIdentifier GroupID;

        int Available = 0;
        /**
         * indirect pointer into which component slot should be taken next
         * Contains the slot as id and version is always INVALID
         */
        EntityID NextAvailableComponent;

        vector<int> GetTargetIndices(const vector<EntityID>& IDs) const
        {
            vector<int> TargetIndices;
            for (const EntityID& ID : IDs)
            {
                TargetIndices.push_back(GetTargetIndex(ID));
            }
            return TargetIndices;
        }
        
        static unsigned int GetIndexInPage(const EntityID& ID)
        {
            return ID.GetID() % SparseSetPage::PAGE_SIZE;
        }
        
        static unsigned int GetBasePageIndex(const EntityID& ID)
        {
            return ID.GetID() / SparseSetPage::PAGE_SIZE;
        }

        
        int GetPageIndex(const EntityID& ID) const
        {
            const auto PageIndex = GetBasePageIndex(ID);
            if (!PageMapping.contains(PageIndex))
                return SparseSetPage::INVALID_INDEX;

            return PageMapping.at(PageIndex);
        }
        
        int GetPageIndex(const EntityID& ID, bool bShouldCreate)
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
        
        void CreatePageAtMapping(int Index)
        {
            assert(Index >= 0 && Index < NumPages);
            int FreeIndex = static_cast<int>(Pages.size());
            Pages.emplace_back();
            Pages[FreeIndex].Init();
            PageMapping[Index] = FreeIndex;
        }
                
        bool Has(const EntityID& ID, const SparseSetPage& Page, int IndexInPage) const
        {
            int TargetIndex = Page.Indices[IndexInPage];
            bool bIsValid = TargetIndex != SparseSetPage::INVALID_INDEX;
            if (!bIsValid || !Components.Has(TargetIndex))
                return false;

            return Components.GetID(TargetIndex) == ID;
        }
        
        int GetFreeValueIndex()
        {
            //assert(!Has(NextAvailable));
            int TargetIndex = NextAvailableComponent.GetID();
            NextAvailableComponent = Components.GetID(TargetIndex);
            --Available;
            return TargetIndex;
        }
        
        void MoveData()
        {
            size_t OldLength = Components.Count;
            size_t NewLength = Components.GetGreaterCount();
            NewLength = min(NewLength, static_cast<size_t>(SparseSetPage::PAGE_SIZE * NumPages));
            Components.ChangeSize(NewLength);
            SetupEmptyRedirectors(OldLength, NewLength);
        }
        
        void SetupEmptyRedirectors(size_t Start, size_t End)
        {
            Components.SetupEmptyRedirectors(Start, End);
            NextAvailableComponent = EntityID::Invalid(static_cast<int>(Start));
            Available += (End - Start);
        }
        
        
    };

}
