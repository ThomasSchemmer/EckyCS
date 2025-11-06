#pragma once

namespace EckyCS
{
    /** Identifies a single Entity, but can indicate multiple versions of it */
    class EntityID
    {
        // split into 24bit ID and 8 bit versions, so max ID is 16.777.216
        unsigned int Data;
        
    public:
        EntityID() = default;
        EntityID(const int ID, const unsigned int Version = 0)
        {
            SetID(ID);
            SetVersion(Version);
        }
        
        EntityID(const size_t ID, const unsigned int Version = 0)
        {
            SetID(static_cast<int>(ID));
            SetVersion(Version);
        }

        unsigned int GetID() const
        {
            return Data >> ID_OFFSET;
        }
        
        void SetID(int ID) {
            Data = ((ID << ID_OFFSET) & ID_MASK) | (Data & VERSION_MASK);
        }

        unsigned int GetVersion() const
        {
            return Data & VERSION_MASK;
        }
        
        void SetVersion(unsigned int Version)
        {
            Data = (Version & VERSION_MASK) | (Data & ID_MASK);
        }

        bool operator==(const EntityID& Other) const
        {
            return GetID() == Other.GetID() &&
                GetVersion() == Other.GetVersion();
        }

        unsigned int operator/(int Other) const
        {
            return GetID() / Other;
        }
        
        bool operator<(const EntityID& other) const noexcept {
            return GetID() < other.GetID();
        }

        bool IsInvalid() const
        {
            return GetVersion() == INVALID;
        }

        void Invalidate()
        {
            SetVersion(INVALID);
        }

        static EntityID Invalid(const int ID) 
        {
            EntityID E(ID, 0);
            E.Invalidate();
            return E;
        }

        static EntityID Invalid(const size_t ID) 
        {
            return Invalid(static_cast<int>(ID));
        }
        
        static EntityID Invalid()
        {
            EntityID E(0, 0);
            E.Invalidate();
            return E;
        }

        static unsigned int INVALID;

    private:

        static unsigned int ID_OFFSET;
        static unsigned int ID_MASK;
        static unsigned int VERSION_MASK;
    };

    
}
