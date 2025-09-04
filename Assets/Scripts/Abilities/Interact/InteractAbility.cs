using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Interact", menuName = "ScriptableObjects/Abilities/Interact", order = 0)]
public class InteractAbility : GameplayAbility
{
    public List<IInteractableAbility> Interactables = new();
    private GameplayAbility LastAbility;

    public override void Tick(float Delta)
    {
        base.Tick(Delta);

        if (LastAbility == null)
            return;

        Debug.Log("Harveest is ready");
        if (!Input.GetKeyDown(ActivationKey))
            return;

        LastAbility.Activate();
    }

    public override bool ShouldActivate()
    {
        LastAbility = null;
        // todo: refactor with base.CanActivate(), just removed key
        bool bIsOffCooldown = CurrentCooldown == -1 || CurrentCooldown == 0;
        bool bIsGranted = Status >= State.Granted;
        bool bHasTags = AssignedToBehaviour.HasAllTags(ActivationRequiredTags.IDs);
        if (!(bIsOffCooldown && bIsGranted && bHasTags))
            return false;

        return CanInteract(out LastAbility);
    }

    private bool CanInteract(out GameplayAbility Ability)
    {
        Ability = default;
        foreach (var Current in Interactables)
        {
            if (!Current.CanInteract())
                continue;

            Ability = Current as GameplayAbility;
            return true;
        }
        return false;
    }

    public override void OnGranted()
    {
        base.OnGranted();
        AssignedToBehaviour.GetGrantedAbilities().ForEach(A => OnGrantedAbility(A));
        AssignedToBehaviour._OnAbilityGranted += OnGrantedAbility;
        AssignedToBehaviour._OnAbilityRemoved += OnRemovedAbility;
    }

    private void OnGrantedAbility(GameplayAbility Ability)
    {
        if (Ability is not IInteractableAbility IAbility)
            return;

        if (Interactables.Contains(IAbility))
            return;
        
        Interactables.Add(IAbility);
    }


    private void OnRemovedAbility(GameplayAbility Ability)
    {
        if (Ability is not IInteractableAbility IAbility)
            return;

        if (Interactables.Contains(IAbility))
            return;

        Interactables.Add(IAbility);
    }

    public override void OnRemoved()
    {
        base.OnRemoved();
        AssignedToBehaviour._OnAbilityGranted -= OnGrantedAbility;
        AssignedToBehaviour._OnAbilityRemoved -= OnRemovedAbility;
    }
}
