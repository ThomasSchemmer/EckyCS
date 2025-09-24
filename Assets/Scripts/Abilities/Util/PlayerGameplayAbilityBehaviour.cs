using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerGameplayAbilityBehaviour : GameplayAbilityBehaviour
{
    public int Index;

    public override bool IsInputDown(InputSettings.Inputs InputType)
    {
        if (!Game.TryGetService(out InputManager InputManager))
            return false;

        return InputManager.IsInputDown(Index, InputType);
    }
}
