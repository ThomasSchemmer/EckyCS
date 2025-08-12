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
        if (!Game.TryGetService(out ECS ECS))
            return;

        CreatePlantAt(new(6, 0, -6));
        CreatePlantAt(new(12, 0, -9));
        CreatePlantAt(new(3, 0, -3));
        CreatePlantAt(new(-3, 0, -6));

        CreatePlantAt(new(12, 0, 3));
        CreatePlantAt(new(0, 0, -9));
        CreatePlantAt(new(3, 0, -12));
        CreatePlantAt(new(6, 0, -15));
        CreatePlantAt(new(0, 0, -15));

        CreatePlantAt(new(-3, 0, -15));
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

        if (!EntityGenerator.TryCreate(out Plant _, Data))
            return;

        Debug.Log("Created Plant");
    }

}
