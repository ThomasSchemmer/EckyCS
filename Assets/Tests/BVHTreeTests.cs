using NUnit.Framework;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.TestTools;
using UnityEngine.UIElements;

public class BVHTreeTests
{

    [Test]
    public unsafe void GlobalTest()
    {
        Init();

        for (int i = 0; i < 100; i++)
        {
            EntityGenerator.TryCreate<TestEntity>(out var Entity, GetTransformCompData());
        }

        if (!Game.TryGetService(out EckyCS ECS))
            return;

        if (!ECS.TryGetSystem(out LocationSystem LocSys))
            return;

        LocSys.FixedTick(0);
        LocSys.IsEntityAt<TransformComponent>(
            new Vector3(2, 0, 2)
        );
        LocSys.Tick(0);
        LocSys.Tick(0);
        LocSys.Tick(0);

    }

    private void Init()
    {
        GameObject ECSObj = new();
        var Game = ECSObj.AddComponent<Game>();
        var ECS = ECSObj.AddComponent<EckyCS>();
        Game.Services.Add(new()
        {
            TargetScript = ECS
        });
        Game.Init();
        ECS.StartService();
    }

    private byte[] GetTransformCompData()
    {
        byte[] Data = new byte[ComponentAllocator.GetSize(typeof(TransformComponent))];
        var TmpX = BitConverter.GetBytes(UnityEngine.Random.Range(0, 10f));
        var TmpY = BitConverter.GetBytes(0f);
        var TmpZ = BitConverter.GetBytes(UnityEngine.Random.Range(0, 10f));

        var Start = 0;
        var Length = sizeof(float);
        Array.Copy(TmpX, 0, Data, Start, Length);
        Array.Copy(TmpY, 0, Data, Start + Length, Length);
        Array.Copy(TmpZ, 0, Data, Start + Length * 2, Length);
        return Data;
    }

}
