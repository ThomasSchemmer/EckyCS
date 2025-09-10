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

    public enum ItemType {
        DEFAULT,
        // fabric
        Flax,
        Cotton,
        Silk,
        // Foods
        Corn,
        Wheat,
        // Fruits
        Apple,
        Grape,
        Strawberry,
        // Ores
        Copper,
        Tin,
        Iron,
        // Woods
        Balsa,
        Oak,
        Walnut,
        Ebony
    }

}
