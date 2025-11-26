#pragma once
#include <iostream>

#include "SparseSetPage.h"
#include "../Components/ComponentGroup.h"
#include "../Util/TypeInfo.h"


namespace EckyCS
{
    /**
     * non-templated baseclass to allow storing sets in a map
     * Subclasses should overwrite/provide better template matching
     *
    * 
     * C++ cannot infer ISparseSet -> SparseSet<Types...>, so we have to do our own lookup
     * when we cast back from the map to call SparseSet.Add(...)
     * We simply store the actual pointer mapping inside this wrapper, so we can go from void**->Types... again
     */
    class ISparseSet
    {
    public:
        virtual ~ISparseSet() = default;
        bool Has(const EntityID& ID) const;
        /** Removes all data of ID */
        virtual void Remove(const EntityID& ID);
        /** Removes all data of IDs */
        virtual void RemoveRange(const vector<EntityID>& IDs);

        virtual size_t GetCount() const;
        virtual size_t GetTotalCount() const;
        virtual bool IsFull() const;
        /** Returns the dense Index for a given ID */
        size_t GetTargetIndex(const EntityID& ID) const;

        virtual void Add(const EntityID& ID) {}
        virtual void AddRange(const vector<EntityID>& IDs) {}

        template <typename T>
        requires is_base_of_v<Component, T>
        /** Moves the components to a target pos in the data */
        void SetData(const EntityID& ID, T& Value)
        {
            if (!ParamLookup.contains(typeid(T)) || !Has(ID))
                return;

            int Offset = ParamLookup[typeid(T)];
            int Size = sizeof(T);
            SetData(ID, Offset, Size, &Value);
        }
        
        template <typename... SubTypes>
        /** Moves the components to a target pos in the data */
        void SetData(const EntityID& ID, SubTypes&... Value)
        {
            (SetData(ID, Value), ...);
        }

        template <typename T>
        requires is_base_of_v<Component, T>
        /** Directly sets data for the given Component, from a specific Start */
        void SetDataBlock(size_t StartIndex, size_t Count, T* ValuePtr)
        {
            if (!ParamLookup.contains(typeid(T)))
                return;

            int Offset = ParamLookup[typeid(T)];
            int Size = sizeof(T);
            SetDataBlock(Offset, Size, StartIndex, Count, ValuePtr);
        }

        template <typename... SubTypes>
        /** Directly fills data for each passed in component */
        void SetDataBlock(size_t StartIndex, size_t Count, SubTypes*... ValuePtrs)
        {
            (SetDataBlock(StartIndex, Count, ValuePtrs), ...);
        }

        template <typename SubType>
        /** Returns the requested type with reinterpreted data*/
        span<SubType> GetSpan(const EntityID& ID)
        {
            const TypeInfo Type = typeid(SubType);
            if (!Has(ID) || !ParamLookup.contains(Type))
                return {};

            int Offset = ParamLookup[Type];
            int Size = sizeof(SubType);
            auto Temp = GetData(ID, Offset, Size);
            auto Bytes = as_writable_bytes(Temp);
            return span(reinterpret_cast<SubType*>(Bytes.data()), Bytes.size() / sizeof(SubType));
        }

        template <typename SubType>
        /** Returns the requested type with reinterpreted data*/
        span<SubType> GetSpan()
        {
            const TypeInfo Type = typeid(SubType);
            if (!ParamLookup.contains(Type))
                return {};

            int Offset = ParamLookup[Type];
            int Size = sizeof(SubType);
            auto Temp = GetData(Offset, Size);
            auto Bytes = as_writable_bytes(Temp);
            return span(reinterpret_cast<SubType*>(Bytes.data()), Bytes.size() / sizeof(SubType));
        }

        template <typename... SubTypes>
        View<SubTypes...> GetData(EntityID& ID)
        {
            auto IDs = span(&ID, 1);
            return make_tuple(IDs, GetSpan<SubTypes>(ID)...);
        }

        template <typename... SubTypes>
        View<SubTypes...> GetData()
        {
            auto IDs = GetIDSpan();
            return make_tuple(IDs, GetSpan<SubTypes>()...);
        }

        template <typename ... Components>
        requires AllComponents<Components...>
        void ForEach(EntityAction<Components...>& Action)
        {
            // have to make a lvalue for the ref
            auto Tmp = GetData<Components...>();
            Action(GroupID, GetTotalCount(), Tmp);
        }
    
    protected:
        vector<SparseSetPage> Pages;
        map<size_t, size_t> PageMapping;
        size_t NumPages = 0;
        ComponentGroupIdentifier GroupID;
        map<TypeInfo, int> ParamLookup;

        size_t Available = 0;

        virtual EntityID RemoveInternal(size_t Index);
        virtual bool HasInternal(const EntityID& ID, const SparseSetPage& Page, size_t IndexInPage) const;
        virtual void SetData(const EntityID&ID, int Offset, int Size, void* Ptr);
        virtual void SetDataBlock(int Offset, int Size, size_t StartIndex, size_t Count, void* Ptr);
        virtual span<Component> GetData(const EntityID& ID, size_t Offset, size_t Size);
        virtual span<Component> GetData(size_t Offset, size_t Size);
        virtual span<EntityID> GetIDSpan();

        vector<size_t> GetTargetIndices(const vector<EntityID>& IDs) const;
        static unsigned int GetIndexInPage(const EntityID& ID);
        static unsigned int GetBasePageIndex(const EntityID& ID);
        size_t GetPageIndex(const EntityID& ID) const;
        size_t GetPageIndex(const EntityID& ID, bool bShouldCreate);
        void CreatePageAtMapping(size_t Index);
    };

}
