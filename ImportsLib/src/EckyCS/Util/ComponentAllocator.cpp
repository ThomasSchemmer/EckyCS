#include "ComponentAllocator.h"
#include "../Components/Component.h"

namespace EckyCS
{
    int ComponentAllocator::CURRENT_COMP_INDEX = 0;
    std::map<std::string, int> ComponentAllocator::TypeToID;
    std::map<int, std::string> ComponentAllocator::IDToType;

    string ComponentAllocator::Register(const shared_ptr<Component>& Component)
    {
        string Type = typeid(Component.get()).name();
        if (TypeToID.contains(Type))
            return Type;

        return Register(Type);
    }

    string ComponentAllocator::Register(const std::string& Type)
    {
        if (TypeToID.contains(Type))
            return Type;
        
        TypeToID[Type] = ++CURRENT_COMP_INDEX;
        IDToType[TypeToID[Type]] = Type;
        return Type;
    }

    int ComponentAllocator::GetIDFor(const shared_ptr<Component>& Component)
    {
        auto&& Type = Register(Component);
        return TypeToID[Type];
    }

    int ComponentAllocator::GetIDFor(const std::string& Name)
    {
        if (!TypeToID.contains(Name))
            return -1;
        
        return TypeToID[Name];
    }

    string ComponentAllocator::GetTypeFor(int ID)
    {
        if (!IDToType.contains(ID))
            return "";
        
        return IDToType[ID];
    }

    
}
