using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

[RequireComponentType(
    typeof(GrowthComponent), 
    typeof(TransformComponent),
    typeof(SpriteComponent)
)]
[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public class Plant : Entity
{
}
