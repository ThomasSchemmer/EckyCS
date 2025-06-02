using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class AbilityBarScreen : MonoBehaviour
{
    public Material AbilityMat;
    private List<AbilityScreen> AbilityScreens = new();

    void Start()
    {
        Game.RunAfterServiceInit((GameplayAbilitySystem GAS) =>
        {
            GAS.RunAfterBehaviourRegistered(GameplayAbilitySystem.Type.Player, CheckForPlayer);
        });
    }

    private void CheckForPlayer(GameplayAbilityBehaviour Behaviour)
    {
        if (Behaviour.Type != GameplayAbilitySystem.Type.Player)
            return;

        GameplayAbilitySystem._OnBehaviourRegistered -= CheckForPlayer;
        CreateAbilities(Behaviour);
    }

    private void CreateAbilities(GameplayAbilityBehaviour PlayerBehaviour)
    {
        if (!Game.TryGetService(out IconFactory Icons) || PlayerBehaviour == null)
            return;

        int TotalWidth = AbilityCount * SlotWidth + (AbilityCount - 1) * SlotOffset;
        RectTransform Rect = transform as RectTransform;
        Rect.sizeDelta = new(TotalWidth, SlotWidth);

        var Abilities = PlayerBehaviour.GetGrantedAbilities();

        for (int i = 0; i < AbilityCount; i++)
        {
            bool bHasAbility = i < Abilities.Count;
            Ability Ability = (Ability)(bHasAbility ? Abilities[i] : null);
            GameObject AbilityGO = Icons.GetVisualsForAbility(transform, Ability);

            RectTransform AbilityRect = AbilityGO.GetComponent<RectTransform>();
            AbilityRect.sizeDelta = new(SlotWidth, SlotWidth);
            AbilityRect.anchoredPosition = new(i * SlotWidth + (i - 1) * SlotOffset, 0);

            AbilityScreen AbilityScreen = AbilityGO.transform.GetChild(0).GetComponent<AbilityScreen>();
            AbilityScreen.Initialize(Ability, AbilityMat);
            AbilityScreens.Add(AbilityScreen);
        }
    }

    private void OnDestroy()
    {
        GameplayAbilitySystem._OnBehaviourRegistered -= CheckForPlayer;
    }

    public static int AbilityCount = 4;
    public static int SlotWidth = 125;
    public static int SlotOffset = 5;
}
