#pragma once
#include <functional>
#include <span>
#include <assert.h>

#include "ComponentGroupIdentifier.h"
#include "../Util/EntityID.h"
#include "../Util/EckyCSHeader.h"

namespace EckyCS
{
    using namespace std;

    template<AllComponents ... Types>
    class ComponentGroup
    {
    public:
        size_t Count;
        ComponentGroupIdentifier GroupID;
        

        ComponentGroup(size_t ExpectedCount) : Count(ExpectedCount)
        {
            GroupID.AddFlags<Types...>();
            IDs = new EntityID[ExpectedCount]();
            SetupEmptyRedirectors(0, Count);
            size_t DataSize = TotalByteCount<Types...>() * Count;
            Data = new byte[DataSize];
        }

        ~ComponentGroup()
        {
            delete[] IDs;
            delete[] Data;
        }

        template<class T>
        requires IsOneOf<T, Types...>
        /** Returns a span<T> for the specified TargetType, allowing for easy access */
        span<T> Get()
        {
            auto Ptr = GetPtrTo<T>();
            return span<T>(Ptr, Count);
        }
        
        /** Returns a tuple of refs for all Types, allowing for easy access */
        View<Types...> Get(int TargetIndex) 
        {
            return Get<Types...>(TargetIndex);
        }

        template <typename... TargetTypes>
        requires AllContainedIn<tuple<TargetTypes...>, tuple<Types...>>
        /** Returns a tuple of refs for all Types, allowing for easy access */
        View<TargetTypes...> Get(size_t TargetIndex)
        {
            assert(TargetIndex >= 0 && TargetIndex < Count);
            return make_tuple(GetIDSpan(), GetSpan<TargetTypes>(TargetIndex)...);
        }

        template <typename... TargetTypes>
        requires AllContainedIn<tuple<TargetTypes...>, tuple<Types...>>
        /** Returns a collection of span<T>, one for each specified TargetType */
        View<TargetTypes...> GetAll()
        {
            return make_tuple(GetIDSpan(), GetSpan<TargetTypes>()...);
        }
        

        template <auto Method, typename System>
        /**
         * Wrapper function that executes the passed in Method on System
         * Automatically deduces necessary types via function template arguments
         */
        void ForEach(System& system)
        {
            using MethodT = decltype((Method));
            using TargetTuple = typename ExtractViewArgs<MethodT>::Types;

            // Forward to a helper to unpack the tuple
            ForEach_Helper<Method>(system, TargetTuple{});
        }

        bool IsValid() const
        {
            return Count > 0 && Data != nullptr && IDs != nullptr;
        }

        size_t GetGreaterCount() const
        {
            size_t Additional = static_cast<size_t>(Count * 0.5);
            return Additional + Count;
        }

        bool Has(int TargetIndex) const
        {
            return !IDs[TargetIndex].IsInvalid(); 
        }

        EntityID& GetID(int TargetIndex) const
        {
            return IDs[TargetIndex];
        }

        void ChangeSize(size_t NewCount)
        {
            ChangeSizeIDs(NewCount);
            ChangeSizeComponents(NewCount);
            for (size_t i = Count; i < NewCount; i++)
            {
                Reset(i);
            }
            Count = NewCount;
        }

        /* Resets the target ID at a specific index and can reset components */
        void Reset(size_t Index, bool bResetComponents = true)
        {
            // bring back "point to next entity in row"
            Set(Index, EntityID::Invalid(Index + 1), bResetComponents);
        }

        /* Sets the target ID at a specific index and can reset components */
        void Set(size_t TargetIndex, const EntityID& ID, bool bResetComponents = true)
        {
            assert(TargetIndex >= 0 && TargetIndex < Count);
            IDs[TargetIndex] = ID;
            if (bResetComponents)
            {
                ResetComponents(TargetIndex);
            }
        }

        template<class T>
        requires IsOneOf<T, Types...>
        /** Moves the component to a target pos in the data */
        void SetData(int TargetIndex, T& Value)
        {
            auto Ptr = GetPtrTo<T>();
            *(Ptr + TargetIndex) = Value;
        }
        
        template <typename... SubTypes>
        requires AllContainedIn<tuple<SubTypes...>, tuple<Types...>>
        /** Moves the components to a target pos in the data */
        void SetData(int TargetIndex, SubTypes&... Values)
        {
            (SetData<SubTypes>(TargetIndex, Values), ...);
        }
        
        template <typename T>
        requires IsOneOf<T, Types...>
        /** Fills every byte at the targeted types and indices with the lowest byte of data*/
        void FillDataAt(size_t TargetIndex, int InData)
        {
            T* TargetOffset = GetPtrTo<T>(TargetIndex);
            memset(TargetOffset, InData, sizeof(T));
        }

        /** Overwrites all component data at TargetIndex with 0 */
        void ResetComponents(size_t TargetIndex)
        {
            (FillDataAt<Types>(TargetIndex, 0), ...);
        }
        
        void SetupEmptyRedirectors(size_t Start, size_t End)
        {
            for (size_t i = Start; i < End; i++)
            {
                Reset(i, false);
            }
        }

        void Swap(size_t IndexA, size_t IndexB)
        {
            swap(IDs[IndexB], IDs[IndexA]);
            swap(Get<Types...>(IndexA), Get<Types...>(IndexA));
        }

        
#ifdef _TEST_BUILD //defined in projects "Test" C++->Preprocessor settings
        byte* GetData() const {return Data;}
        EntityID* GetIDs() const {return IDs;}
#endif
        
    private:
        /**
         * Raw data pointer to root of all components
         * Since we want to store all data contiguous in memory, we cant use
         * std::tuple, so use a byte[] instead
         * Any access should only be used with the Get<T>() functions, or through Actions
         */
        byte* Data;
        EntityID* IDs;

        template <auto Method, typename System, typename... T>
        void ForEach_Helper(System& system, std::tuple<T...>)
        {
            EntityAction<T...> F = Bind<Method, System, T...>(system);
            this->template ForEach<T...>(F);
        }

        template <auto Method, typename System, typename... TargetTypes>
        requires AllContainedIn<tuple<TargetTypes...>, tuple<Types...>>
        /**
         * Helper function to create a Lambda around a specific System obj
         */
        static auto Bind(System& system)
        {            
            return [&system](auto&&... args)
            {
                (system.*Method)(std::forward<decltype(args)>(args)...);
            };
        }
        
        template <typename... Targets>
        requires AllContainedIn<tuple<Targets...>, tuple<Types...>>
        /**
         * Function wrapper that provides pointers to all the different components
         * Used to enforce a strict way of writing functions that work on Entities
         */
        using EntityAction = function<void(ComponentGroupIdentifier, EntityID, View<Targets...>&)>;

        template <typename... TargetTypes>
        requires AllContainedIn<tuple<TargetTypes...>, tuple<Types...>>
        /**
         * Actual implementation to iterate over all requested views
         * Since this would require a tedious bind/cast of functions to
         * adhere to the calling standard, we use the other functions to automatically
         * morph in between
         */
        void ForEach(EntityAction<TargetTypes...>& Action)
        {
            auto View = GetAll<TargetTypes...>();
            for (size_t i = 0; i < Count; i++)
            {
                if (IDs[i].IsInvalid())
                    continue;
                
                Action(GroupID, IDs[i], View);
            }
        }
        
        template<class T>
        requires IsOneOf<T, Types...>
        /** Returns the span of a type (so offset from data origin)*/
        span<T> GetSpan()
        {
            return span<T>(GetPtrTo<T>(), Count);
        }

        template<class T>
        requires IsOneOf<T, Types...>
        /** Returns the span of a type, but for a specific index*/
        span<T> GetSpan(size_t TargetIndex)
        {
            return span<T>(GetPtrTo<T>(TargetIndex), 1);
        }
        
        span<EntityID> GetIDSpan()
        {
            return {IDs, Count};
        }
        
        template<class T>
        requires IsOneOf<T, Types...>
        /** Returns the pointer to the first element of a type (so offset from data origin)*/
        T* GetPtrTo()
        {
            constexpr size_t Index = GetIndexOf<T, Types...>();
            size_t Offset = SumSizesTo<Index, Types...>();
            return reinterpret_cast<T*>(Data + Offset);
        }
        
        template<class T>
        requires IsOneOf<T, Types...>
        /** Returns the pointer to the selected index of a type*/
        T* GetPtrTo(size_t TargetIndex)
        {
            return GetPtrTo<T>() + TargetIndex;
        }
            
        void ChangeSizeIDs(size_t NewCount)
        {
            EntityID* NewIDs = new EntityID[NewCount]();
            std::copy_n(IDs, Count, NewIDs);
            delete[] IDs;
            IDs = NewIDs;
        }

        void ChangeSizeComponents(size_t NewCount)
        {
            byte* NewData = new byte[NewCount]();
            std::copy_n(Data, Count, NewData);
            delete[] Data;
            Data = NewData;
        }

    };

    

}
