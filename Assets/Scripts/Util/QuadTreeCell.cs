using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

partial class QuadTree
{
    private struct QuadTreeCell
    {
        public EntityID FirstElement;
        // TODO: Minimize, can prolly reduce overall size by 30-40%
        // TODO: since its static we can compute on the fly
        public Vector3 Center;
        public int ParentID;
        public int FirstChildID;
        public int ID;
        public int PlaceableCount;
        public byte Depth;

        public override string ToString()
        {
            return Center.ToString() + " : " + PlaceableCount;
        }
    }
}