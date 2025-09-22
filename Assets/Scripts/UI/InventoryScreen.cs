using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class InventoryScreen : MonoBehaviour
{
    public RectTransform Container;

    private NumberedIconScreen[] Screens;

    private InventoryService Service;
    private RSIconFactory Icons;

    public void Start()
    {
        Game.RunAfterServicesInit((RSIconFactory Icons, InventoryService Service) =>
        {
            this.Service = Service;
            this.Icons = Icons;

            Vector2Int Width = ScreenOffset + ScreenSize;
            int Amount = (Size.x / Width.x) * (Size.y / Width.y);
            Screens = new NumberedIconScreen[Amount];
            for (int i = 0; i < Amount; i++)
            {
                InitScreen(Icons, i);
            }
        });
    }

    private void InitScreen(RSIconFactory Icons, int i)
    {
        var Screen = Icons.GetVisualsForNumberedIcon(Container, i);
        Screen.transform.SetParent(Container, false);
        var Icon = Screen.transform.GetChild(0).GetComponent<RectTransform>();
        Icon.sizeDelta = ScreenSize;
        Icon.anchoredPosition = new();
        Screens[i] = Screen;
    }

    private void ShowItem(int i)
    {
        bool bHasItem = Service.TryGetItemInfoAt(i, out var Type, out var Amount);
        Screens[i].SetEnabled(bHasItem);
        if (!bHasItem)
        {
            Screens[i].ClearInfo();
            return;
        }

        Screens[i].Initialize(Icons.GetIconForItem(Type), "", null);
    }

    public void Toggle()
    {
        SetVisibility(!gameObject.activeSelf);
    }

    public void SetVisibility(bool bShouldShow)
    {
        gameObject.SetActive(bShouldShow);
        if (!bShouldShow)
            return;

        for (int i = 0; i < Screens.Length; i++)
        {
            ShowItem(i);
        }
    }

    private Vector2Int ScreenSize = new(100, 100);
    private Vector2Int ScreenOffset = new(15, 15);
    private Vector2Int Size = new(950, 550);
}
