using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraController : MonoBehaviour
{

    public float Sensitity = 400;
    public float Smoothness = 1;
    public bool bInjectPlayer = false;
    public Transform Player;
    public Texture2D CursorTex;
    private MovementController PlayerController;

    private float OffsetMag;
    private float OffsetHeight;
    private Vector3 TargetRotation = new();

    public void Start()
    {
        Cursor.lockState = bInjectPlayer ? CursorLockMode.Locked : CursorLockMode.None;
        Cursor.visible = !bInjectPlayer;

        OffsetMag = Vector3.Magnitude(transform.position - Player.position);
        OffsetHeight = transform.position.y - Player.position.y;
        PlayerController = Player.GetComponent<MovementController>();
    }

    private void UpdateCamera()
    {
        transform.position = Player.position - OffsetMag * transform.forward + new Vector3(0, OffsetHeight * Player.localScale.y, 0);
        float MouseX = Input.GetAxisRaw("Mouse X") * Time.deltaTime * Sensitity;
        float MouseY = Input.GetAxisRaw("Mouse Y") * Time.deltaTime * Sensitity;

        TargetRotation += new Vector3(-MouseY, MouseX, 0);
        float X = Mathf.Clamp(TargetRotation.x, -35, 35);
        TargetRotation = new(X, TargetRotation.y, TargetRotation.z);
        transform.rotation = Quaternion.Lerp(transform.rotation, Quaternion.Euler(TargetRotation), Time.deltaTime * Smoothness);
        Player.rotation = Quaternion.Euler(new(0, TargetRotation.y, 0));
    }

    public void Update()
    {
        UpdateCamera();
    }

}
