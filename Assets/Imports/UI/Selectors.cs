

using UnityEngine;

/**
 * Helper class to wrap all templated selectors into one easily accesible UI element / gameservice
 */
public class Selectors : GameService
{

    protected override void StartServiceInternal()
    {
        UISelector = new Selector<UIElement>(true);

        UISelector.Layer = UILayerName;
        Game.Instance._OnPause += OnPause;
        Game.Instance._OnResume += OnResume;
        Game.Instance._OnPopup += OnPopup;

        _OnInit?.Invoke(this);
    }

    protected override void StopServiceInternal() { }

    public void Update()
    {
        if (!bIsEnabled || !IsInit)
            return;

        if (UISelector.RayCast())
            return;

        if (bIsPopuped)
            return;
    }

    private void OnPause()
    {
        bIsEnabled = false;
        ForceDeselect();
    }

    private void OnPopup(bool bIsOpen)
    {
        bIsPopuped = bIsOpen;
        if (bIsPopuped)
        {
            ForceDeselect();
        }
    }

    private void OnResume()
    {
        bIsEnabled = true;
    }

    public UIElement GetSelectedUIElement()
    {
        return UISelector.Selected;
    }

    public void ForceDeselect()
    {
        DeselectUI();
        HideTooltip();
    }

    public void DeselectUI()
    {
        UISelector.DeSelect(false);
        UISelector.DeSelect(true);
    }

    public void ShowTooltip(ISelectable Selectable, bool bShow)
    {
        ToolTipScreen.Show(Selectable, bShow);
    }

    public void HideTooltip()
    {
        ToolTipScreen.Show(null, false);
    }

    public Selector GetSelectorByType(ISelectable Selectable)
    {
        if (Selectable is UIElement)
            return UISelector;

        return null;
    }
    protected override void ResetInternal()
    {
    }

    public Selector<UIElement> UISelector;
    public ToolTipScreen ToolTipScreen;

    private bool bIsEnabled = true;
    private bool bIsPopuped = false;

    public static string UILayerName = "UI";
}