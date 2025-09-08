using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public abstract class GameplayAbilityInstancingPolicy : ScriptableObject
{
    public enum Instancing
    {
        InstancedPerExecution,
        InstancedPerActor,
        NonInstanced
    }

    public Instancing InstancingPolicy;

    public Object GetByInstancing(GameplayAbilityBehaviour Target)
    {
        switch (InstancingPolicy)
        {
            case Instancing.NonInstanced: return this;
            case Instancing.InstancedPerActor: return GetByInstancingActor(Target);
            case Instancing.InstancedPerExecution: return Instantiate(this);
        }
        return null;
    }

    public void DeleteByInstancing()
    {
        switch (InstancingPolicy)
        {
            case Instancing.NonInstanced: return;
            default: Destroy(this); break;
        }
    }

    private Object GetByInstancingActor(GameplayAbilityBehaviour Target)
    {
        if (Target.GetActiveEffects().Contains(this))
            return this;

        return Instantiate(this);
    }
}
