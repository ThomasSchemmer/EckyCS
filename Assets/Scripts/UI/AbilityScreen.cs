using System.Collections;
using System.Collections.Generic;
using Unity.VectorGraphics;
using UnityEngine;
using UnityEngine.UIElements;

[RequireComponent(typeof(SVGImage))]
public class AbilityScreen : MonoBehaviour
{
    public GameplayAbility Ability;

    private SVGImage Image;
    private SVGImage Background;

    public void Initialize(GameplayAbility Ability, Material Mat, int Size)
    {
        Background = transform.parent.GetComponent<SVGImage>();
        Image = GetComponent<SVGImage>();
        Image.material = Instantiate(Mat);
        RectTransform Rect = GetComponent<RectTransform>();
        Rect.sizeDelta = new(Size, Size);
        Vector4 PosSize = new(
            Rect.position.x - Screen.width / 2f,
            Rect.position.y - Screen.height / 2f,
            Rect.sizeDelta.x,
            Rect.sizeDelta.y
        );
        Image.material.SetVector("_PosSize", PosSize);
        this.Ability = Ability;
        CreateIcon();
    }

    private void CreateIcon()
    {
        if (!Game.TryGetService(out RSIconFactory Icons))
            return;

        Image.sprite = Icons.GetIconForAbility(Ability != null ? Ability.Type : AbilityType.DEFAULT);
        Image.enabled = Image.sprite != null;
    }

    private void Update()
    {
        if (Ability == null) 
            return;

        Image.material.SetFloat("_Cutoff", Ability.GetCooldownCutoff());
        Background.color = IsActive() ? ActiveColor : InactiveColor;
    }

    private bool IsActive()
    {
        if (Ability == null)
            return false;

        return Ability.Status >= GameplayAbility.State.Activated && Ability.Status <= GameplayAbility.State.Ended;
    }

    private static Color ActiveColor = new(1, 1, .5f, 1);
    private static Color InactiveColor = new(1, 1, 1, 1);

}
