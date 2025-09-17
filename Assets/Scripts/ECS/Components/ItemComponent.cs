using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
/** 
 * Holds information about what (droppable, collectable) item this is
 */
public struct ItemComponent : IComponent
{
    public ItemType Type;
    public uint Amount;
}
