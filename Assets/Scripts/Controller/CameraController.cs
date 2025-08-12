using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraController : MonoBehaviour
{

    public bool bInjectPlayer = false;
    public Transform Player;

    private float OffsetMag;

    public void Start()
    {
        Cursor.lockState = bInjectPlayer ? CursorLockMode.Locked : CursorLockMode.None;
        Cursor.visible = !bInjectPlayer;

        OffsetMag = Vector3.Magnitude(transform.position - Player.position);
    }

    private void UpdateCamera()
    {
        transform.position = Player.position - OffsetMag * transform.forward;
    }

    public void Update()
    {
        UpdateCamera();
    }

}
