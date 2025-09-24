using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class AbilityBar : MonoBehaviour
{
    public Material AbilityMat;
    public int PlayerIndex = -1;
    private List<AbilityScreen> AbilityScreens = new();

    public void Init(int PlayerIndex)
    {
        this.PlayerIndex = PlayerIndex;
        Game.RunAfterServiceInit((GameplayAbilitySystem GAS) =>
        {
            GAS.RunAfterBehaviourRegistered(GameplayAbilitySystem.Type.Player, CreateAbilities, false);
        });
    }

    private void CreateAbilities(GameplayAbilityBehaviour Behaviour)
    { 
        if (Behaviour is not PlayerGameplayAbilityBehaviour PlayerBehavior)
            return;

        if (PlayerBehavior.Index != PlayerIndex)
            return;

        var Abilities = new List<GameplayAbility>();
        Abilities.AddRange(Behaviour.GetGrantedAbilities());
        for (int i = Abilities.Count - 1; i >= 0; i--)
        {
            if (!Abilities[i].bIsHidden)
                continue;

            Abilities.RemoveAt(i);
        }

        CreateAbilities(Abilities);

        if (!Game.TryGetService(out GameplayAbilitySystem GAS))
            return;

        GAS.RemoveBehaviourRegisteredCallback(GameplayAbilitySystem.Type.Player, CreateAbilities);
    }

    private void CreateAbilities(List<GameplayAbility> Abilities)
    {
        if (!Game.TryGetServices(out RSIconFactory Icons, out PlayerInstantiatorService Players))
            return;

        GameObject ParentUI = GameObject.Find("UI");
        transform.SetParent(ParentUI.transform, false);

        int PlayerCount = Players.GetPlayerCount();
        int AbilityCount = Mathf.Min(Abilities.Count, AbilityBar.AbilityCount);
        int Width = PlayerCount == 2 ? SlotWidthSmall : SlotWidth;
        int TotalWidth = AbilityCount * Width + (AbilityCount - 1) * SlotOffset;
        RectTransform Rect = transform as RectTransform;
        Rect.sizeDelta = new(TotalWidth, Width);
        int CenterX =
            PlayerCount == 1 ? Screen.width / 2 :
            PlayerIndex == 0 ? Screen.width / 3 :
            Screen.width / 3 * 2;
        Rect.anchoredPosition = new(CenterX, SlotPosY);

        for (int i = 0; i < AbilityCount; i++)
        {
            bool bHasAbility = i < Abilities.Count;
            GameplayAbility Ability = (bHasAbility ? Abilities[i] : null);
            if (Ability == null)
                continue;

            Ability.Input = (InputSettings.Inputs)((int)InputSettings.Inputs.Ability0 + i);
            GameObject AbilityGO = Icons.GetVisualsForAbility(transform, Ability);

            RectTransform AbilityRect = AbilityGO.GetComponent<RectTransform>();
            AbilityRect.sizeDelta = new(Width, Width);
            AbilityRect.anchoredPosition = new(i * Width + (i - 1) * SlotOffset, 0);

            AbilityScreen AbilityScreen = AbilityGO.transform.GetChild(0).GetComponent<AbilityScreen>();
            AbilityScreen.Initialize(Ability, AbilityMat, Width - 10);
            AbilityScreens.Add(AbilityScreen);
        }
    }

    private void OnDestroy()
    {
    }

    public const int AbilityCount = 4;
    public const int SlotPosY = 100;
    public const int SlotWidth = 125;
    public const int SlotWidthSmall = 75;
    public const int SlotOffset = 5;
}
