using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using UnityEngine;
using UnityEngine.UIElements;

[CreateAssetMenu(fileName = "Shovel", menuName = "ScriptableObjects/Abilities/Util/Shovel", order = 0)]
public class Shovel : GameplayAbility
{
    public GameObject Preview;


    public override void Tick(float Delta)
    {
        base.Tick(Delta);
        Vector3 Position = GetPlantPosition();
        Preview.transform.position = Position;

        if (Status != State.Committed)
            return;

        if (!Input.GetMouseButtonDown(0))
            return;

        if (IsPlantAt())
            return;

        CreatePlantAt(GetPlantPosition());
    }

    private bool IsPlantAt()
    {
        if (!Game.TryGetService(out ECS ECS))
            return true;

        if (!ECS.TryGetSystem(out LocationSystem LocSys))
            return true;

        return LocSys.IsEntityAt<GrowthComponent, TransformComponent>(GetPlantPosition());
    }

    private Vector3 GetPlantPosition()
    {
        Vector3 Temp = AssignedToBehaviour.transform.position + AssignedToBehaviour.transform.forward;
        Temp.x = Mathf.RoundToInt(Temp.x / GridSize) * GridSize;
        Temp.y = Mathf.RoundToInt(Temp.y / GridSize) * GridSize;
        Temp.z = Mathf.RoundToInt(Temp.z / GridSize) * GridSize;
        return Temp;
    }

    private void CreatePlantAt(Vector3 Position)
    {
        // ugly data formatting, otherwise we have to iterate through ECS
        // TODO: should prolly be made an actual function but will prolly use
        // reflection for datatypes :/
        byte[] Data = new byte[EntityGenerator.GetSize<Plant>()];
        var TmpX = BitConverter.GetBytes(Position.x);
        var TmpY = BitConverter.GetBytes(Position.y);
        var TmpZ = BitConverter.GetBytes(Position.z);

        var Start = EntityGenerator.GetOffsetOf<Plant>(typeof(TransformComponent));
        var Length = sizeof(float);
        Array.Copy(TmpX, 0, Data, Start, Length);
        Array.Copy(TmpY, 0, Data, Start + Length, Length);
        Array.Copy(TmpZ, 0, Data, Start + Length * 2, Length);

        if (!EntityGenerator.TryCreate(out Plant Plant, Data))
            return;
    }


    public override void OnGranted()
    {
        base.OnGranted();
        _OnActivateAbility += CreatePreview;
        _OnEndAbility += DestroyPreview;
    }

    public override void OnRemoved()
    {
        base.OnRemoved();
        _OnActivateAbility -= CreatePreview;
        _OnEndAbility -= DestroyPreview;
    }

    private void CreatePreview()
    {
        Preview = GameObject.CreatePrimitive(PrimitiveType.Cube);
    }
    private void DestroyPreview()
    {
        DestroyImmediate(Preview);
    }


    public const int GridSize = 3;
}
