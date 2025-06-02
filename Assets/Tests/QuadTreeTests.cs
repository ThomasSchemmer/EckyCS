using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

public class QuadTreeTests
{
    [Test]
    public void DeleteSwapTest()
    {
        QuadTree QuadTree = new();
        QuadTree.Init(ECS.MaxEntities);
        int MaxDepth = QuadTree.MaxDepth;
        float Range = QuadTree.GlobalRange;
        float MinRange = Range * Mathf.Pow(0.5f, MaxDepth);
        float NextMinRange = Range * Mathf.Pow(0.5f, MaxDepth - 1);

        TransformComponent A = new()
        {
            PosX = Range - MinRange
        };
        TransformComponent B = new()
        {
            PosX = Range - MinRange - NextMinRange * 2
        };
        EntityID E0 = new(0);
        EntityID E1 = new(1);
        QuadTree.Add(ref E0, ref A);
        QuadTree.Add(ref E1, ref B);
        QuadTree.Delete(ref E0);
        Assert.IsTrue(QuadTree.Verify());
    }

    [Test]
    public void SelfParentTest()
    {

        QuadTree QuadTree = new();
        QuadTree.Init(ECS.MaxEntities);

        TransformComponent A = new()
        {
            PosX = -0.8788047f,
            PosZ = 0.8534851f
        };
        TransformComponent B = new()
        {
            PosX = -0.6337261f,
            PosZ = 0.8286991f
        };
        TransformComponent C = new()
        {
            PosX = 0.8751769f,
            PosZ = -0.852988f
        };
        EntityID E0 = new(0);
        EntityID E1 = new(1);
        EntityID E2 = new(2);
        QuadTree.Add(ref E0, ref A);
        QuadTree.Add(ref E1, ref B);
        QuadTree.Add(ref E2, ref C);

        for (int i = 0; i < 10; i++)
        {
            A.PosX += 1;
            A.PosZ += 1;
            QuadTree.Add(ref E0, ref A);

            B.PosX += 1;
            B.PosZ += 1;
            QuadTree.Add(ref E1, ref B);

            C.PosX += 1;
            C.PosZ += 1;
            QuadTree.Add(ref E2, ref C);
        }
    }

}
