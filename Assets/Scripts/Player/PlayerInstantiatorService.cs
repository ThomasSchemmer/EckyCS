using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInstantiatorService : GameService
{
    public bool bDynamicView = false;
    public GameObject PlayerPrefab;
    public GameObject CameraPrefab;
    public GameObject AbilityBarPrefab;
    public Camera BaseCam;

    public enum ViewState
    {
        Single,
        StaticSplit,
        DynamicSplit
    }

    private List<PlayerGameplayAbilityBehaviour> PlayerBehaviours = new();
    private List<PlayerController> PlayerControllers = new();
    private List<AbilityBar> AbilityBars = new();
    private List<Camera> PlayerCameras = new();

    public (Vector3, Vector3) GetCamPos()
    {
        Vector3 Cam0Pos = PlayerCameras[0].transform.position;
        Vector3 Cam1Pos = GetPlayerCount() == 2 ?
            PlayerCameras[1].transform.position :
            Vector3.zero;

        return (Cam0Pos, Cam1Pos);
    }

    public ViewState GetViewState()
    {
        if (GetPlayerCount() < 2)
            return ViewState.Single;

        return bDynamicView ? ViewState.DynamicSplit : ViewState.StaticSplit;
    }

    public PlayerController GetPlayerController(int i)
    {
        return PlayerControllers[i];
    }

    public int GetPlayerCount()
    {
        return PlayerControllers.Count;
    }

    private void Instantiate(int i)
    {
        GameObject Player = Instantiate(PlayerPrefab);

        HandleGAS(Player, i);
        HandleController(Player, i);
        HandleCam(Player, i);
        HandleAbilityBar(Player, i);
        //      -Abilities
        //services  
        //  inventory
        // Splitscreeen
    }

    private void HandleAbilityBar(GameObject Player, int i)
    {
        AbilityBar Bar = Instantiate(AbilityBarPrefab).GetComponent<AbilityBar>();
        Bar.Init(i);
        AbilityBars.Add(Bar);
    }

    private void HandleController(GameObject Player, int i)
    {
        PlayerControllers.Add(Player.GetComponent<PlayerController>());
    }

    private void HandleGAS(GameObject Player, int i)
    {
        var PlayerGAB = Player.GetComponent<PlayerGameplayAbilityBehaviour>();
        PlayerGAB.Index = i;
        PlayerBehaviours.Add(PlayerGAB);
        PlayerBehaviours[i].Attributes = Instantiate(Resources.Load(AttributeLocation) as AttributeSet);
    }

    private void HandleCam(GameObject Player, int i)
    {
        BaseCam.enabled = false;

        var Cam = Instantiate(CameraPrefab).GetComponent<Camera>();
        PlayerCameras.Add(Cam);
        Cam.GetComponent<CameraController>().Player = Player.transform;

        Cam.tag = i == 0 ? "MainCamera" : "Untagged";
        // force main cam to be rendered last, ensuring we always have an overlay
        Cam.depth = i == 0 ? 1 : 0;
        Cam.GetComponent<AudioListener>().enabled = i == 0;
    }

    protected override void ResetInternal()
    {
        //todo: kill
    }

    protected override void StartServiceInternal()
    {
        for (int i = 0; i < Game.TargetPlayerCount; i++)
        {
            Instantiate(i);
        }
        _OnInit?.Invoke(this);
    }

    protected override void StopServiceInternal()
    {
    }

    private const string AttributeLocation = "GAS/PlayerAttributes";
}
