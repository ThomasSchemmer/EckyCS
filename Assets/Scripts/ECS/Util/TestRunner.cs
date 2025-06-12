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


        for (int i = 0; i < 100; i++) //ECS.MaxEntities
        {
            EntityGenerator.TryCreate(out Enemy _);
        }
        //ECS.AssignComponent<NameComponent>(new EntityID(0));
        ECS.AddSystem(new LocationSystem());
    }

}
