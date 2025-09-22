using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RSGameplayAbilitySystem : GameplayAbilitySystem
{
    protected override void StartServiceInternal()
    {
        Game.RunAfterServiceInit((PlayerInstantiatorService Players) =>
        {
            base.StartServiceInternal();
        });
    }
}
