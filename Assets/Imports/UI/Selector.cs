using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public abstract class Selector
{
    public enum RaycastType { 
        UIOnly,
        WorldOnly
    }

    public void SetSelected(ISelectable Target, bool bSelected)
    {
        if (bSelected)
        {
            Select(Target);
        }
        else
        {
            DeSelect();
        }
    }

    public void SetHovered(ISelectable Target, bool bHovered)
    {
        if (bHovered)
        {
            Hover(Target);
        }
        else
        {
            DeHover();
        }
    }

    public abstract void Select(ISelectable Target);
    public abstract void Hover(ISelectable Target);

    public abstract void DeSelect();
    public abstract void DeHover();
}

/**
 * Class that automates selecting gameobjects with the mouse easy. Checks by type if a hovered object is selectable
 * and calls the Selectable interface accordingly
 */
public class Selector<T> : Selector where T : ISelectable
{
    public Selector(bool bIsUIOnly = false)
    {
        Type = bIsUIOnly ? RaycastType.UIOnly : RaycastType.WorldOnly;
        MainCam = Camera.main;
    }

    public bool RayCast()
    {
        bool bIsLeftClick = Input.GetMouseButtonDown(0);
        bool bIsRightClick = Input.GetMouseButtonDown(1) && !bIsLeftClick;
        bool bIsEscapeClick = Input.GetKeyDown(KeyCode.Escape);

#if UNITY_EDITOR
        if (Input.GetKeyDown(KeyCode.E))
        {
            Debug.Log("");
        }
#endif

        if (bIsEscapeClick || !RayCast(out GameObject Hit))
        {
            DeSelect(bIsLeftClick);
            return false;
        }

        if (!Hit)
        {
            // can only be true if Unity UI has been hit (eg a button). Simply swallow the input
            return true;
        }

        T Target = TryGetTargetFrom(Hit);

        if (Target == null)
        {
            DeSelect(bIsLeftClick);
            return false;
        }

        if (bIsRightClick)
        {
            Target.Interact();
            return true;
        }

        if (bIsLeftClick)
        {
            // we do intentionally not return, since this event should trigger on every click
            Target.ClickOn(GetPointerData().position);
        }

        if ((Selected != null && bIsLeftClick && Target.IsEqual(Selected)) || (Hovered != null && !bIsLeftClick && Target.IsEqual(Hovered)))
        {
            // we still hit something, even if its still the old selectable
            LongHover();
            return true;
        }

        DeSelect(bIsLeftClick);
        if (bIsLeftClick)
        {
            Select(Target);
        }
        else
        {
            Hover(Target);
        }
        return true;
    }

    public override void Select(ISelectable Target)
    {
        if (!Target.CanBeInteracted())
            return;

        Selected = (T)Target;
        Target.SetSelected(true);

        OnItemSelected?.Invoke(Selected);
        _OnSelected.ForEach(_ => _.Invoke(Selected));
    }

    public override void Hover(ISelectable Target)
    {
        if (!Target.CanBeInteracted())
            return;

        Hovered = (T)Target;
        HoverPosition = Input.mousePosition;
        Target.SetHovered(true);
        if (OnItemHovered != null)
        {
            OnItemHovered(Hovered);
        }
    }

    public void DeSelect(bool bIsClicked)
    {
        if (bIsClicked)
        {
            DeSelect();
        }
        else
        {
            DeHover();
        }
    }

    public override void DeSelect()
    {
        if (Selected == null)
            return;

        Selected.SetSelected(false);
        Selected = default(T);
        if (OnItemDeSelected != null)
        {
            OnItemDeSelected();
        }
    }

    public override void DeHover()
    {
        if (Hovered == null)
            return;

        Hovered.SetHovered(false);
        Hovered = default(T);
        if (OnItemDeHovered != null)
        {
            OnItemDeHovered();
        }
        StopLongHover();
    }

    private T TryGetTargetFrom(GameObject Hit)
    {
        T HitTarget = Hit.GetComponent<T>();
        if (HitTarget != null)
            return HitTarget.ShouldBeIgnored() ? default : HitTarget;

        Transform HitTransform = Hit.transform;
        while (HitTransform.parent != null && HitTarget == null)
        {
            HitTransform = HitTransform.parent;
            HitTarget = HitTransform.GetComponent<T>();
        }

        if (HitTarget == null || HitTarget.ShouldBeIgnored())
            return default;

        return HitTarget;
    }

    private void LongHover()
    {
        if (Hovered == null)
            return;

        if (!Hovered.CanBeLongHovered())
            return;

        if (Vector2.Distance(HoverPosition, Input.mousePosition) > 100)
        {
            StopLongHover();
            return;
        }

        HoverTimeS += Time.deltaTime;
        HoverPosition = Input.mousePosition;
        if (HoverTimeS < LongHoverTimeS)
            return;

        if (bShowHover)
            return;

        if (!Game.TryGetService(out Selectors Selectors))
            return;

        bShowHover = true;
        Selectors.ShowTooltip(Hovered, true);
    }

    private void StopLongHover()
    {
        HoverTimeS = 0;
        HoverPosition = Vector2.zero;
        if (!bShowHover)
            return;

        bShowHover = false;
        if (!Game.TryGetService(out Selectors Selectors))
            return;

        Selectors.ShowTooltip(null, false);
    }

    private bool RayCast(out GameObject Hit)
    {
        Hit = null;
        switch (Type)
        {
            case RaycastType.UIOnly: return RayCastUI(out Hit);
            case RaycastType.WorldOnly: return RayCastWorld(out Hit);
            default: return false;
        }
    }

    private bool RayCastWorld(out GameObject HitObj)
    {
        HitObj = null;
        Rect ScreenRect = new Rect(0, 0, Screen.width, Screen.height);
        if (!ScreenRect.Contains(Input.mousePosition))
            return false;

        Ray Ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        bool bHasHit = Physics.Raycast(Ray, out var Hit);
        bool bIsVisible = bHasHit;
        bool bIsValid = bHasHit && bIsVisible;

        HitObj = bIsValid ? Hit.collider.gameObject : null;

        return bIsValid;
    }

    private bool RayCastUI(out GameObject Hit)
    {
        Hit = null;

        List<RaycastResult> Hits = new List<RaycastResult>();
        EventSystem.current.RaycastAll(GetPointerData(), Hits);

        if (Hits.Count == 0)
            return false;

        foreach (RaycastResult Result in Hits)
        {
            if (Result.gameObject.layer == LayerMask.NameToLayer(Layer))
            {
                Hit = Result.gameObject;
                return true;
            }
        }

        // we return "hit with no object" to indicate that we are hovering over unity base-ui 
        // and should swallow follup selectors
        Hit = null;
        return true;
    }


    static PointerEventData GetPointerData()
    {
        return new PointerEventData(EventSystem.current)
        {
            position = Input.mousePosition
        };
    }

    public RaycastType Type = RaycastType.WorldOnly;
    public T Selected;
    public T Hovered;
    public float LongHoverTimeS = 1f;

    public string Layer;

    private float HoverTimeS = 0;
    private bool bShowHover = false;
    private Vector2 HoverPosition = Vector2.zero;
    private Camera MainCam;

    public delegate void _ItemInteracted(T Item);
    public delegate void _ItemNotInteracted();
    public event _ItemInteracted OnItemSelected;
    public event _ItemNotInteracted OnItemDeSelected;
    public event _ItemInteracted OnItemHovered;
    public event _ItemNotInteracted OnItemDeHovered;


    public ActionList<T> _OnSelected = new();
}
