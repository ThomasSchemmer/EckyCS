#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../Util/ActionMap.h"
#include "Abilities/AbilityType.h"
#include "Tags/GameplayTagMask.h"


namespace GAS
{
    class AttributeSet;
    class GameplayAbility;
    class GameplayEffect;
    class GameplayTagContainer;
    class GameplayAbilitySystem;
    using namespace std;
    using namespace GameImports;
    
    enum class GameplayAbilityComponentType : uint8_t
    {
        DEFAULT,
        Player
    };
    
    /**
     * Handles GAS interactions per object,
     * can execute @GameplayAbility's and check @Attribute's
     */
    class GameplayAbilityComponent : public enable_shared_from_this<GameplayAbilityComponent>
    {
        // for calling @AddEffect
        friend class GameplayAbilitySystem;
    public:
        GameplayAbilityComponentType Type = GameplayAbilityComponentType::DEFAULT;
        shared_ptr<AttributeSet> Attributes;

        vector<shared_ptr<GameplayAbility>> GrantedOnStart;
        vector<shared_ptr<GameplayEffect>> ActiveEffects;
        vector<shared_ptr<GameplayEffect>> MarkedForRemovalEffects;
        map<AbilityType, shared_ptr<GameplayAbility>> GrantedAbilities;
        map<AbilityType, ActionMap<AbilityType, shared_ptr<GameplayAbility>>> OnAbilityGrantedCallbacks;
        
        GameplayAbilityComponent();
        virtual ~GameplayAbilityComponent();
        void Init();
        void Tick(float Delta);
        void AddTag(const string& Tag);
        void RemoveTag(const string& Tag);
        void AddTags(const vector<string>& Tags);
        void RemoveTags(const vector<string>& Tags);
        bool HasTag(const string& Tag);
        bool HasAllTags(const vector<string>& Tags);
        bool HasAnyTags(const vector<string>& Tags);

        vector<shared_ptr<GameplayEffect>> GetActiveEffects();
        bool TryGetAnyActiveEffectsByTags(const vector<string>& Tags, vector<shared_ptr<GameplayEffect>>& FoundEffects);
        bool TryGetAnyActiveEffectByTag(const string& Tag, shared_ptr<GameplayEffect>& FoundEffect);
        void RemoveEffectByTag(const string& Tag);
        void RemoveEffect(const shared_ptr<GameplayEffect>& Effect);

        void GrantAbility(const shared_ptr<GameplayAbility>& Ability);
        void RemoveAbility(const shared_ptr<GameplayAbility>& Ability);
        vector<shared_ptr<GameplayAbility>> GetGrantedAbilities() const;
        bool HasAbility(shared_ptr<GameplayAbility> Ability) const;
        void RunAfterAbilityGranted(AbilityType AbilityType, GameImports::Action<shared_ptr<GameplayAbility>>& Callback);

        ActionMap<AbilityType, shared_ptr<GameplayAbility>> OnAbilityGranted;
        ActionMap<AbilityType, shared_ptr<GameplayAbility>> OnAbilityRemoved;
        ActionMap<GameplayAbilityComponentType, GameplayTagMask&> OnTagsChanged;
        ActionMap<GameplayAbilityComponentType, const string&> OnTagAdded;
        ActionMap<GameplayAbilityComponentType, const string&> OnTagRemoved;
        
    private:
        bool bIsInitialized = false;
        map<shared_ptr<GameplayEffect>, bool> EffectsToHandle;
        GameplayTagMask ActiveTags;

        void AddEffect(shared_ptr<GameplayEffect> Effect);
        
        void TickAbilities(float Delta) const;
        void TickEffects(float Delta);
        void HandleDelayedEffects();
        void HandleOnStartAbilities();
    };
}
