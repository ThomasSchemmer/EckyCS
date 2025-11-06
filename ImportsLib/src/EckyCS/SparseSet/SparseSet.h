#pragma once
#include "ISparseSet.h"
#include "SparseSetPage.h"
#include "../Components/ComponentGroup.h"

namespace EckyCS
{
    class ComponentGroupIdentifier;
    
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
    class SparseSet : public ISparseSet
    {
        
    public:
        
        SparseSet(int InNumPages = 1, int ExpectedEntities = 10) :
            Components(ExpectedEntities)
        {
            GroupID.AddFlags<Types...>();
            NumPages = InNumPages;
            Pages.reserve(NumPages);
            SetupEmptyRedirectors(0, ExpectedEntities);
            RegisterParamOffsets();
        }
        
        
        int GetCount() const override 
        {
            return Components.Count - Available;
        }
        
        void Add(const EntityID& ID) override
        {
            size_t PageIndex = GetPageIndex(ID, true);
            if (PageIndex == SparseSetPage::INVALID_INDEX)
                return;
            
            size_t Index = GetIndexInPage(ID);
            auto& Page = Pages[PageIndex];
            if (HasInternal(ID, Page, Index))
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
        }

        

        template <typename ... SubTypes>
        requires AllContainedIn<tuple<SubTypes...>, tuple<Types...>>
        void SetData(const EntityID& ID, SubTypes&... Values)
        {
            if (!Has(ID))
                return;

            Components.SetData(GetTargetIndex(ID), Values...);
        }
        
        View<Types...> GetData(const EntityID& ID) 
        {
            if (!Has(ID))
                return View<Types...>();

            int TargetIndex = GetTargetIndex(ID);
            return Components.Get(TargetIndex);
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
        /** Executes the Method on each Entity, one by one*/
        void ForEachEntity(System& system)
        {
            Components.template ForEachEntity<Method>(system);
        }
        //template <auto Method, typename System>
        //void CheckEntity(System& system);
        //template <auto Method, typename System>
        //EntityID SelectEntityFrom(const vector<EntityID>& IDs, System& system);
        //template <auto Method, typename System>
        //void ForEachEntityFrom(const vector<EntityID>& IDs, System& system);

    protected:
        ComponentGroup<Types...> Components;
        
        /**
         * indirect pointer into which component slot should be taken next
         * Contains the slot as id and version is always INVALID
         */
        EntityID NextAvailableComponent;

        template <typename... SubTypes>
        requires AllContainedIn<tuple<SubTypes...>, tuple<Types...>>
        /** Moves the components to a target pos in the data */
        void SetDataInternal(const EntityID& ID, SubTypes&... Values)
        {
            auto Index = GetTargetIndex(ID);
            if (Index == SparseSetPage::INVALID_INDEX)
                return;

            Components.SetData(Index, Values...);
        }
        
        bool HasInternal(const EntityID& ID, const SparseSetPage& Page, size_t IndexInPage) const override
        {
            size_t TargetIndex = Page.Indices[IndexInPage];
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
        
        EntityID RemoveInternal(size_t Index) override
        {
            // actually want a copy here
            EntityID Temp = NextAvailableComponent;
            NextAvailableComponent = EntityID(Index, EntityID::INVALID);
            Components.Reset(Index);
            Components.Set(Index, Temp, false); //already reset
            return Temp;
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

        /**
         * This registers the offset for each of the Types... into a lookup, so that thee
         * Baseclass @ISparseSet can directly check if it contains a type and we
         * don't have to overload a templated function ala "Has<Type>()"
         */
        void RegisterParamOffsets()
        {
            auto handle = [&](auto I)
            {
                using Target = TypeAt_t<I, Types...>;
                ParamLookup.emplace(typeid(Target), SumSizesTo<I, Types...>());
            };

            [&]<size_t... I>(index_sequence<I...>)
            {
                (handle(integral_constant<size_t, I>{}), ...);
            }(make_index_sequence<sizeof...(Types)>());
        }

        
        span<Component> GetData(const EntityID& ID, size_t Offset, size_t Size) override
        {
            const size_t TargetIndex = GetTargetIndex(ID);
            return Components.GetData(TargetIndex, Offset, Size);
        }

        void SetData(const EntityID& ID, int Offset, int Size, void* Value) override
        {
            const size_t TargetIndex = GetTargetIndex(ID);
            const size_t PtrToComp = Offset * Components.Count;
            const size_t PtrInsideComp = TargetIndex * Size;
            Components.SetData(PtrToComp + PtrInsideComp, Size, Value);
        }
    };

}
