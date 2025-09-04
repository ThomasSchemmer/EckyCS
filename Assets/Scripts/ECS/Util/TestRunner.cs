using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Profiling;
using UnityEngine.Rendering.VirtualTexturing;

public class TestRunner : MonoBehaviour
{

    void Start()
    {

        ECSTest();

    }


    private void ECSTest() {
        if (!Game.TryGetService(out EckyCS ECS))
            return;

        ECS.AddSystem(new GrowthSystem());
        return;
        for (int i = 0; i < 5; i++)
        {
            CreatePlantAt(new(
                UnityEngine.Random.Range(-5, 5),
                0,
                UnityEngine.Random.Range(-5, 5)
            ));
        }
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
        var bGrowth = BitConverter.GetBytes(0);

        var Start = EntityGenerator.GetOffsetOf<Plant>(typeof(TransformComponent));
        var Length = sizeof(float);
        var StartGrowth = EntityGenerator.GetOffsetOf<Plant>(typeof(GrowthComponent));
        var LengthGrowth = sizeof(int);
        Array.Copy(TmpX, 0, Data, Start, Length);
        Array.Copy(TmpY, 0, Data, Start + Length, Length);
        Array.Copy(TmpZ, 0, Data, Start + Length * 2, Length);
        Array.Copy(bGrowth, 0, Data, StartGrowth, LengthGrowth);

        if (!EntityGenerator.TryCreate(out Plant _, Data))
            return;
    }

}
