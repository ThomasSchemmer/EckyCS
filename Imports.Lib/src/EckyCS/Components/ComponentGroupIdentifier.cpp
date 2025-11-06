#include "ComponentGroupIdentifier.h"

#include "../Util/ComponentAllocator.h"

namespace EckyCS
{
    
    ComponentGroupIdentifier::ComponentGroupIdentifier(const BitVector& NewFlags)
    {
        this->Flags = NewFlags;
    }

    void ComponentGroupIdentifier::AddFlag(const string& Name)
    {
        int Index = ComponentAllocator::GetIDFor(Name);
        if (Index < 0)
        {
            // lazy register
            auto FinalName = ComponentAllocator::Register(Name);
            Index = ComponentAllocator::GetIDFor(FinalName);
        }
        Flags.Set(Index, true);
    }

    void ComponentGroupIdentifier::AddFlag(const shared_ptr<Component>& Ptr)
    {
        AddFlag(GetComponentName(Ptr));
    }

    void ComponentGroupIdentifier::AddFlags(const vector<string>& Names)
    {
        for (auto&& Name : Names)
        {
            AddFlag(Name);
        }
    }

    void ComponentGroupIdentifier::AddFlags(ComponentList Names)
    {
        AddFlags(GetComponentNames(Names));
    }

    bool ComponentGroupIdentifier::HasFlag(const string& Name) const
    {
        const int Index = ComponentAllocator::GetIDFor(Name);
        return Flags.Get(Index);
    }

    bool ComponentGroupIdentifier::HasFlag(const shared_ptr<Component>& Ptr) const
    {
        return HasFlag(GetComponentName(Ptr));
    }

    bool ComponentGroupIdentifier::HasAllFlags(const vector<string>& Names) const
    {
        for (auto&& Name : Names)
        {
            if (HasFlag(Name))
                continue;

            return false;
        }
        return true;
    }

    bool ComponentGroupIdentifier::HasAllFlags(ComponentList Names) const
    {
        return HasAllFlags(GetComponentNames(Names));
    }

    bool ComponentGroupIdentifier::HasAnyFlag(const vector<string>& Names) const
    {
        for (auto&& Name : Names)
        {
            if (!HasFlag(Name))
                continue;

            return true;
        }
        return false;
    }

    bool ComponentGroupIdentifier::HasAnyFlag(ComponentList Names) const
    {
        return HasAnyFlag(GetComponentNames(Names));
    }

    void ComponentGroupIdentifier::RemoveFlag(const string& Name)
    {
        const int Index = ComponentAllocator::GetIDFor(Name);
        Flags.Set(Index, false);
    }

    void ComponentGroupIdentifier::RemoveFlag(const shared_ptr<Component>& Ptr)
    {
        RemoveFlag(GetComponentName(Ptr));
    }

    int ComponentGroupIdentifier::GetAmountOfFlags() const
    {
        return Flags.GetAmountSetBits();
    }

    bool ComponentGroupIdentifier::operator<(const ComponentGroupIdentifier& other) const noexcept
    {
        return Flags < other.Flags;
    }

    ComponentGroupIdentifier ComponentGroupIdentifier::Clone() const
    {
        ComponentGroupIdentifier Clone(this->Flags);
        return Clone;
    }

    ComponentGroupIdentifier ComponentGroupIdentifier::Subtract(const ComponentGroupIdentifier& Other) const
    {
        return ComponentGroupIdentifier(Flags.Subtract(Other.Flags));
    }

    bool ComponentGroupIdentifier::operator==(const ComponentGroupIdentifier& Other) const
    {
        return this->Flags == Other.Flags;
    }

    
    int ComponentGroupIdentifier::GetSelfIndexOf(const std::string& Name) const
    {
        int Target = ComponentAllocator::GetIDFor(Name);
        return Flags.GetSelfIndexOf(Target);
    }

    int ComponentGroupIdentifier::GetSelfIndexOf(const shared_ptr<Component>& Ptr) const
    {
        return GetSelfIndexOf(GetComponentName(Ptr));
    }

    vector<int> ComponentGroupIdentifier::GetSelfIndicesOf(const vector<string>& Names) const
    {
        vector<int> Result;
        Result.reserve(Names.size());
        for (auto&& Name : Names)
        {
            Result.emplace_back(GetSelfIndexOf(Name));
        }
        return Result;
    }

    vector<int> ComponentGroupIdentifier::GetSelfIndicesOf(ComponentList Names) const
    {
        return GetSelfIndicesOf(GetComponentNames(Names));
    }

    vector<string> ComponentGroupIdentifier::GetContainedTypes() const
    {
        auto&& IDs = Flags.GetAllIndicesWith(true);
        vector<string> Result;
        Result.reserve(IDs.size());
        
        for (auto&& ID : IDs)
        {
            Result.emplace_back(ComponentAllocator::GetTypeFor(ID));
        }
        return Result;
    }

}
