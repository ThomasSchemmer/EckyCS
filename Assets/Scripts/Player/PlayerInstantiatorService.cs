using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInstantiatorService : GameService
{
    public GameObject PlayerPrefab;
    public GameObject CameraPrefab;
    public Camera BaseCam;


    private List<PlayerGameplayAbilityBehaviour> PlayerBehaviours = new();
    private List<PlayerController> PlayerControllers = new();
    private List<Camera> PlayerCameras = new();



    private void Instantiate(int i)
    {
        GameObject Player = Instantiate(PlayerPrefab);

        HandleGAS(Player, i);
        HandleController(Player, i);
        HandleCam(Player, i);
        //      -Abilities
        //UI
        //      - create Bar
        //services  
        //  inventory
        // Splitscreeen
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
        BaseCam.gameObject.SetActive(false);
        var Cam = Instantiate(CameraPrefab).GetComponent<Camera>();
        PlayerCameras.Add(Cam);
        Cam.GetComponent<CameraController>().Player = Player.transform;

        if (i == 0)
        {
            Cam.tag = "MainCamera";
            Cam.GetComponent<AudioListener>().enabled = true;
        }
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
