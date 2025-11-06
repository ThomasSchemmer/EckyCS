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
        virtual void Remove(const EntityID& ID);
        virtual void RemoveRange(const vector<EntityID>& IDs);

        virtual int GetCount() const;
        virtual bool IsFull() const;
        size_t GetTargetIndex(const EntityID& ID) const;

        virtual void Add(const EntityID& ID) {}

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

        template <typename... SubTypes>
        View<SubTypes...> GetData(const EntityID& ID)
        {
            auto IDs = span(&ID, 1);
            return make_tuple(IDs, GetSpan<SubTypes>(ID)...);
        }
    
    protected:
        vector<SparseSetPage> Pages;
        map<size_t, size_t> PageMapping;
        size_t NumPages = 0;
        ComponentGroupIdentifier GroupID;
        map<TypeInfo, int> ParamLookup;

        int Available = 0;

        virtual EntityID RemoveInternal(size_t Index);
        virtual bool HasInternal(const EntityID& ID, const SparseSetPage& Page, size_t IndexInPage) const;
        virtual void SetData(const EntityID&ID, int Offset, int Size, void* Ptr);
        virtual span<Component> GetData(const EntityID& ID, size_t Offset, size_t Size);

        vector<size_t> GetTargetIndices(const vector<EntityID>& IDs) const;
        static unsigned int GetIndexInPage(const EntityID& ID);
        static unsigned int GetBasePageIndex(const EntityID& ID);
        size_t GetPageIndex(const EntityID& ID) const;
        size_t GetPageIndex(const EntityID& ID, bool bShouldCreate);
        void CreatePageAtMapping(size_t Index);
    };

}
