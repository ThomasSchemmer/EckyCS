using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

[RequireComponentType(
    typeof(TransformComponent)
)]
[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public class TestEntity : Entity
{
}
