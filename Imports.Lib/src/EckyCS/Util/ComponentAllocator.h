#pragma once
#include <map>
#include <memory>
#include <string>

namespace EckyCS
{
    using namespace std;
    class ComponentGroupIdentifier;
    class Component;
    class BitVector;
    
    /**
     * Static helper class to map components into their ID
     * Used mainly in bitmasks for easy group compares
     */
    class ComponentAllocator
    {
        // for GetID(string)
        friend ComponentGroupIdentifier;
        friend BitVector;
        
    public:
        
        static std::map<std::string, int> TypeToID;
        static std::map<int, std::string> IDToType;

        static string Register(const shared_ptr<Component>& Component);
        static string Register(const std::string& Type);
        static int GetIDFor(const shared_ptr<Component>& Component);
        static string GetTypeFor(int ID);

        template <typename Type>
        static string GetNameFor()
        {
            return Register(typeid(Type).name());
        }

    private:
        static int GetIDFor(const std::string& Name);

        static int CURRENT_COMP_INDEX;
    };
}
