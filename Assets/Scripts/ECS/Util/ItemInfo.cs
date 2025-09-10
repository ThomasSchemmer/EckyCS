using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/** 
 * Placeholder to combine multiple Item data into a single struct
 * Useful to reduce drawcalls in combination with texture stitching
 * Not serializable, cause it should be hidden
 */
public class ItemInfo : RenderInfo
{
    public Texture2D CombinedTex;
    public Vector4 Size;

    public static Mesh QuadMesh;

    public override Mesh GetMesh()
    {
        if (QuadMesh == null)
        {
            var Obj = Resources.Load("Models/Quad") as GameObject;
            QuadMesh = Obj.GetComponent<MeshFilter>().sharedMesh;
        }
        return QuadMesh;
    }
}
