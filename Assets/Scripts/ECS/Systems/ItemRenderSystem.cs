using System;
using System.Collections;
using System.Collections.Generic;
using Unity.VectorGraphics;
using UnityEngine;

/** 
 * Handles the rendering of all item entities (sprites) that should be displayed
 * lying on the ground. 
 * While a list of individual items is created, its stitched together and then passed along to
 * the sprite shader instead of a single call each time
 */
public class ItemRenderSystem : RenderSystem<ItemRenderData, ItemInfo>
{
    private Texture2D CombinedTex;

    public override void Start()
    {
        // we need to create the merged info before we can use it to initialise
        MergeInfos();
        base.Start();
    }

    private void MergeInfos()
    {
        var Names = Enum.GetNames(typeof(ItemType));
        int CountSide = Mathf.CeilToInt(Mathf.Sqrt(Names.Length));
        CombinedTex = new(CountSide * Width, CountSide * Width, TextureFormat.RGBA32, false);
        CombinedTex.Apply();
        var Test = Resources.LoadAll(ItemLocation);
        for (int i = 0; i < Names.Length; i++) 
        {
            var Tex = Resources.Load(ItemLocation + Names[i] + "tex") as Texture2D;
            if (Tex == null)
                continue;

            int x = i % CountSide;
            int y = i / CountSide;
            // source sprit eis null
            Graphics.CopyTexture(
                Tex, 0, 0, 0, 0, Width, Width,
                CombinedTex, 0, 0, Width * x, Width * y
            );
        }
        CombinedTex.Apply();

        if (Infos.Count == 0)
        {
            Infos.Add(new());
        }
        Infos[0].CombinedTex = CombinedTex;
        Infos[0].Scale = Vector3.one;
        Infos[0].Size = new(Width, Width, CountSide, CountSide);
        Infos[0].ShaderPass = 0;
    }

    public unsafe void Update()
    {
        ECS.GetProvider().Get<TransformComponent, ItemComponent>().ForEachGroup((Group, Ptrs, Count) =>
        {
            int DataTarget = Group.GetSelfIndexOf(typeof(ItemComponent));
            int DataStride = ComponentAllocator.GetSize(typeof(ItemComponent));
            Register(Group, Ptrs, Ptrs[DataTarget], DataStride, Count);
        });
    }

    private const string ItemLocation = "Icons/Items/";
    private const int Width = 256;
}
