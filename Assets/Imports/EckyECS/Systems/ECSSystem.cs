using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface ECSSystem 
{

    // must not be called "Start" to avoid confusing unity
    public abstract void StartSystem();
    public virtual void Tick(float Delta) {}
    public virtual void FixedTick(float FixedDelta) { }
    public virtual void LateTick(float Delta) { }
    public abstract void Destroy();
}
