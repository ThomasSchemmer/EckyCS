#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Util/BitVector.h"
#include "../Util/ComponentAllocator.h"
#include "../Util/EckyCSHeader.h"

namespace EckyCS
{
    using namespace std;
    class Component;

    typedef const vector<shared_ptr<Component>>& ComponentList;
    
    inline string GetComponentName(const shared_ptr<Component>& Ptr)
    {
        return ComponentAllocator::Register(Ptr);
    }
    
    inline vector<string> GetComponentNames(ComponentList Ptrs)
    {
        vector<string> Names;
        Names.reserve(Ptrs.size());
        for (const auto& Ptr : Ptrs)
        {
            Names.emplace_back(ComponentAllocator::Register(Ptr));
        }
        return Names;
    }


    /** Wrapper for BitVector to quickly identify @ComponentGroup by its components*/
    class ComponentGroupIdentifier : enable_shared_from_this<ComponentGroupIdentifier>
    {
    public:
        ComponentGroupIdentifier() = default;
        ComponentGroupIdentifier(const BitVector& NewFlags);
        void AddFlag(const string& Name);
        void AddFlag(const shared_ptr<Component>& Ptr);
        void AddFlags(const vector<string>& Names);
        void AddFlags(ComponentList Names);
        bool HasFlag(const string& Name) const;
        bool HasFlag(const shared_ptr<Component>& Ptr) const;
        bool HasAllFlags(const vector<string>& Names) const;
        bool HasAllFlags(ComponentList Names) const;
        bool HasAnyFlag(const vector<string>& Names) const;
        bool HasAnyFlag(ComponentList Names) const;
        void RemoveFlag(const string& Name);
        void RemoveFlag(const shared_ptr<Component>& Ptr);
        int GetAmountOfFlags() const;
        bool operator<(const ComponentGroupIdentifier& other) const noexcept;

        template<typename... Types>
        requires AllComponents<Types...>
        void AddFlags()
        {
            vector<string> Names = {ComponentAllocator::GetNameFor<Types>() ...};
            AddFlags(Names);
        }
        
        template<typename T>
        requires is_base_of_v<Component, T>
        void AddFlag()
        {
            AddFlag(ComponentAllocator::GetNameFor<T>());
        }
        
        template<typename T>
        requires is_base_of_v<Component, T>
        bool HasFlag() const
        {
            return HasFlag(ComponentAllocator::GetNameFor<T>());
        }

        template<typename... Types>
        requires AllComponents<Types...>
        bool HasAllFlags() const
        {
            return (this->template HasFlag<Types>() && ...);
        }

        ComponentGroupIdentifier Clone() const;
        ComponentGroupIdentifier Subtract(const ComponentGroupIdentifier& Other) const;
        bool operator==(const ComponentGroupIdentifier&) const;

        /** Returns the internal offset of the type inside the flags */
        int GetSelfIndexOf(const std::string& Name) const;
        int GetSelfIndexOf(const shared_ptr<Component>& Ptr) const;
        vector<int> GetSelfIndicesOf(const vector<string>& Names) const;
        vector<int> GetSelfIndicesOf(ComponentList Names) const;
        vector<string> GetContainedTypes() const;
        
    private:
        BitVector Flags;
        
    };
}
