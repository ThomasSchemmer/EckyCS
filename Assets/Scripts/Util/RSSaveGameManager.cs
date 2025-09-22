using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RSSaveGameManager : SaveGameManager
{
    protected override void WaitForRequiredServices()
    {
        //no base call!

        // wait for Factories and GAS
        Game.RunAfterServicesInit((IconFactory IconFactory, MeshFactory MeshFactory) =>
        {
            Game.RunAfterServiceInit((GameplayAbilitySystem GAS) =>
            {
                // will invoke every other "regular" game service to run
                LoadOtherServices();
            });
        });
    }
}
