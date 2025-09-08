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
    public int Range = 1;

    protected override void TickInternal(float Delta)
    {
        base.TickInternal(Delta);
        Vector3 Position = GetPlantPosition();
        Preview.transform.position = Position;

        if (Status != State.Committed)
            return;

        if (!Input.GetMouseButtonDown(0))
            return;

        if (IsPlantAt())
            return;

        Shovel.TryCreatePlantAt(Position, GrowthComponent.PlantType.Wheat, out var Plant);
    }

    private bool IsPlantAt()
    {
        if (!Game.TryGetService(out EckyCS ECS))
            return true;

        if (!ECS.TryGetSystem(out LocationSystem LocSys))
            return true;

        return LocSys.IsEntityAt<GrowthComponent, TransformComponent>(GetPlantPosition(), 0.1f);
    }

    public Vector3 GetPlantPosition()
    {
        // todo: geet heeight from terrain
        Vector3 Temp = AssignedToBehaviour.transform.position + AssignedToBehaviour.transform.forward;
        Temp.x = Mathf.RoundToInt(Temp.x / GridSize) * GridSize;
        Temp.y = 0;
        Temp.z = Mathf.RoundToInt(Temp.z / GridSize) * GridSize;
        return Temp;
    }

    public static bool TryCreatePlantAt(Vector3 Position, GrowthComponent.PlantType Type, out Plant Plant)
    {
        // ugly data formatting, otherwise we have to iterate through ECS
        // TODO: should prolly be made an actual function but will prolly use
        // reflection for datatypes :/

        byte[] Data = new byte[EntityGenerator.GetSize<Plant>()];
        var TmpX = BitConverter.GetBytes(Position.x);
        var TmpY = BitConverter.GetBytes(Position.y);
        var TmpZ = BitConverter.GetBytes(Position.z);

        var StartPos = EntityGenerator.GetOffsetOf<Plant>(typeof(TransformComponent));
        var LengthPos = sizeof(float);
        Array.Copy(TmpX, 0, Data, StartPos, LengthPos);
        Array.Copy(TmpY, 0, Data, StartPos + LengthPos, LengthPos);
        Array.Copy(TmpZ, 0, Data, StartPos + LengthPos * 2, LengthPos);


        var StartGrowth = EntityGenerator.GetOffsetOf<Plant>(typeof(GrowthComponent));
        int Growth = 0;
        int PlantedAtS = (int)Time.realtimeSinceStartup;
        var bGrowth = BitConverter.GetBytes(Growth);
        var bType = BitConverter.GetBytes((int)Type);
        var bPlantedAt = BitConverter.GetBytes(PlantedAtS);
        Array.Copy(bGrowth, 0, Data, StartGrowth, sizeof(int));
        Array.Copy(bType, 0, Data, StartGrowth + sizeof(int), sizeof(GrowthComponent.PlantType));
        Array.Copy(bPlantedAt, 0, Data, StartGrowth + sizeof(int) + sizeof(GrowthComponent.PlantType), sizeof(int));

        return EntityGenerator.TryCreate(out Plant, Data);
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
