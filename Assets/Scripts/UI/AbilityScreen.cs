using System.Collections;
using System.Collections.Generic;
using Unity.VectorGraphics;
using UnityEngine;

[RequireComponent(typeof(SVGImage))]
public class AbilityScreen : MonoBehaviour
{
    public Ability Ability;

    private SVGImage Image;

    public void Initialize(Ability Ability, Material Mat)
    {
        Image = GetComponent<SVGImage>();
        Image.material = Instantiate(Mat);
        RectTransform Rect = GetComponent<RectTransform>();
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
        if (!Game.TryGetService(out IconFactory Icons))
            return;

        Image.sprite = Icons.GetIconForAbility(Ability != null ? Ability.Type : AbilityType.DEFAULT);
        float Alpha = Image.sprite != null ? SlotAlpha : EmptySlotAlpha;
        Image.color = new(1, 1, 1, Alpha);
    }

    private void Update()
    {
        if (Ability == null) 
            return;

        Image.material.SetFloat("_Cutoff", Ability.GetCooldownCutoff());
    }


    public static float SlotAlpha = 1f;
    public static float EmptySlotAlpha = 0.6f;
}
