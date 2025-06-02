using System.Collections;
using System.Collections.Generic;
using Unity.VectorGraphics;
using UnityEngine;

/** 
 * Class representing an icon screen that also contains a number, eg indicating its amount
 */
public class NumberedIconScreen : SimpleIconScreen
{
    private TMPro.TextMeshProUGUI CountText;

    public void OnDestroy()
    {
    }

    public override void Initialize(Sprite Sprite, string HoverTooltip, ISelectable Parent)
    {
        base.Initialize(Sprite, HoverTooltip, Parent);
        Initialize();
    }

    public void Show(bool bShow)
    {
        gameObject.SetActive(bShow);
    }

    private void Initialize()
    {
        CountText = transform.GetChild(1).GetComponent<TMPro.TextMeshProUGUI>();
        float x = CountText.transform.localPosition.x;

        CountText.transform.localPosition = new Vector3(x, 0, 0);
        CountText.alignment = TMPro.TextAlignmentOptions.MidlineLeft;
    }

    public void SetAmountAlignment(TMPro.TextAlignmentOptions Alignment)
    {
        CountText.alignment = Alignment;
    }

    public void UpdateVisuals(int Count, int Max = -1)
    {
        string MaxText = Max >= 0 ? "/" + Max : "";
        CountText.text = "" + Count + MaxText;
    }

    public override void SetSelectionEnabled(bool bEnabled)
    {
        base.SetSelectionEnabled(bEnabled);
        CountText.gameObject.layer = bEnabled ? LayerMask.NameToLayer(Selectors.UILayerName) : 0;
    }

    private static Color ALLOWED_COLOR = new Color(0.2f, 0.4f, 0.2f);
    private static Color FORBIDDEN_COLOR = new Color(0.9f, 0.25f, 0.25f);

}
