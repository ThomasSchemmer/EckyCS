#pragma once
#include <functional>
#include <span>
#include <tuple>
#include <type_traits>

#include "EntityID.h"
#include "../Components/Component.h"

namespace EckyCS
{
    class ComponentGroupIdentifier;
    class EntityID;
    /**
     * Global header that holds useful template meta patterns
     */

    using namespace std;

#define OUT /* out param */
    
    /** Typedef that requires all to inherit from @Component */
    template<typename... Types>
    concept AllComponents = (is_base_of_v<Component, Types> && ...);

    /** Convenience wrapper giving the amount of types in a template */
    template<typename... Types>
    constexpr size_t TypeCount = sizeof...(Types);

    /** Typedef that requires T to be one of Types */
    template <typename T, typename... Types>
    concept IsOneOf = (is_same_v<T, Types> || ...);

    /**
     * Base declaration so the compiler knows it exists:
     * Check if everything IsOneOf<>
     */
    template <typename TargetTuple, typename SupersetTuple>
    struct AllContainedInHelper;

    /** Specialization for Tuples: Check if everything IsOneOf<> */
    template <typename... Targets, typename... Superset>
    struct AllContainedInHelper<std::tuple<Targets...>, std::tuple<Superset...>> {
        static constexpr bool value = (IsOneOf<Targets, Superset...> && ...);
    };

    /**
     * Checks if each type from Target is a type from SuperSet
     * Needs to be called with tuple<Target>, tuple<SuperSet> to actually split the
     * variable length template!
     */
    template <typename TargetTypes, typename SuperSet>
    concept AllContainedIn =
        AllContainedInHelper<TargetTypes, SuperSet>::value;
    
    /** Returns the type at position X */
    template <size_t Index, typename... Types>
    using TypeAt_t = tuple_element_t<Index, tuple<Types...>>;

    /** Returns the total size of all Types defined by the index sequence*/
    template <typename Tuple, size_t... Is>
    constexpr size_t SumSizesIn(index_sequence<Is...>) {
        return (0 + ... + sizeof(tuple_element_t<Is, Tuple>));
    }

    /** Convenience Wrapper to create an index sequence and get the resulting size*/
    template <size_t N, typename... Types>
    constexpr size_t SumSizesTo() {
        return SumSizesIn<tuple<Types...>>(make_index_sequence<N>{});
    }

    /** returns the total size of all classes defined in Types in bytes*/
    template <typename... Types>
    constexpr size_t TotalByteCount()
    {
        return (0 + ... + sizeof(Types));
    }

    /** returns the index of T inside of Types */
    template<class T, typename... Types>
    requires IsOneOf<T, Types...>
    constexpr size_t GetIndexOf()
    {
        size_t Index = 0;
        // uses lazy evaluation to auto-cancel on the first "true"
        ((is_same_v<T, Types> ? true : (++Index, false)) || ...);
        return Index;
    }
    
    /** Overload for tuple types **/
    template<class T, class Tuple>
    constexpr size_t GetIndexOf()
    {
        return []<size_t... I>(std::index_sequence<I...>)
        {
            return GetIndexOf<T, std::tuple_element_t<I, Tuple>...>();
        }(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }

    template <typename... Types>
    requires AllComponents<Types...>
    /**
     * Collection of components and corresponding EntityIDs
     * Note: EntityIDs are always at the FIRST index!
     */
    using View = tuple<span<EntityID>, span<Types>...>;

    template <typename... Targets>
    requires AllComponents<Targets...>
    /**
     * Function wrapper that prosvides pointers to all the different components
     * Used to enforce a strict way of writing functions that work on Entities
     */
    using EntityAction = function<bool(ComponentGroupIdentifier, size_t, View<Targets...>&)>;
    
    template <typename T, typename... Types>
    requires AllComponents<T>
    /** Wrapper to get a single span from a view by type */
    constexpr span<T> Get(View<Types...>& View)
    {
        constexpr size_t Index = GetIndexOf<T, Types...>() + 1;
        return std::get<Index>(View);
    }
    
    template <typename... Types>
    /** Wrapper to get the EntityIDs span from a view*/
    constexpr span<EntityID> GetID(View<Types...>& View)
    {
        return std::get<0>(View);
    }
    
    template <typename Tuple, size_t... I>
    /** Helper pattern that uses a compile-time index sequence via unfolding */
    void* TupleAtImpl(Tuple& tuple, size_t index, std::index_sequence<I...>)
    {
        void* result = nullptr;
        ((index == I ? (result = static_cast<void*>(std::get<I>(tuple).data()), true) : false) || ...);
        return result;
    }

    template <typename Tuple>
    /** Runtime visitor pattern to get the i-th element in a tuple */
    void* TupleAt(Tuple&& tuple, size_t index)
    {
        constexpr size_t N = std::tuple_size_v<std::remove_reference_t<Tuple>>;
        return TupleAtImpl(
            std::forward<Tuple>(tuple),
            index,
            std::make_index_sequence<N>{}
        );
    }

    template <typename>
    struct ExtractViewArgs;
    
    template <typename System, typename... T>
    /** Helps the compiler auto-extract templated types */
    struct ExtractViewArgs<bool (System::*)(ComponentGroupIdentifier, size_t, View<T...>&)>
    {
        using Types = std::tuple<T...>;
    };
    
    template <typename Tuple>
    /** Helper struct to go from Tuple<A, B, C> to A, B, C*/
    struct UnpackTuple;

    template <typename... Ts>
    struct UnpackTuple<std::tuple<Ts...>> {
        template <template <typename...> typename T>
        using apply = T<Ts...>;
    };

}

