using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "PlantSelection", menuName = "ScriptableObjects/GAS/Cues/PlantSelection")]
public class PlantSelectionCue : GameplayAbilityCue
{
    private EckyCS ECS;
    private Harvest Harvest;

    private bool bShouldTick = false;

    public override void OnBeforeAbilityTick()
    {
        SetVisible(false);
    }

    public override void OnAfterAbilityTick()
    {
        SetVisible(true);
    }

    private unsafe void SetVisible(bool bShouldShow)
    {
        if (!bShouldTick || !Harvest)
            return;

        var Targets = Harvest.GetTargets();
        if (Targets == null)
            return;

        foreach (var Pair in Targets)
        {
            if (Pair.Value.Count == 0)
                continue;

            var Set = ECS.GetSet(Pair.Key);
            Set.ForEachEntityFrom(Pair.Value, (Group, Ptrs, Index) =>
            {
                int HighlightIndex = Group.GetSelfIndexOf(typeof(HighlightComponent));
                if (HighlightIndex < 0)
                    return false;

                var Ptr = ((HighlightComponent*)Ptrs[HighlightIndex]) + Index;
                Ptr->IsHighlighted = bShouldShow ? 1u : 0;
                return true;
            });
        }
    }

    protected override void InitInternal(GameplayAbility Ability)
    {
        base.InitInternal(Ability);
        Game.RunAfterServiceInit((EckyCS ECS) =>
        {
            this.ECS = ECS;
        });

        if (AssignedToAbility is not Harvest Harvest)
            return;

        this.Harvest = Harvest;
    }

    protected override void EnableCue()
    {
        bShouldTick = true;
    }

    protected override void DisableCue()
    {
        bShouldTick = false;
    }
}
