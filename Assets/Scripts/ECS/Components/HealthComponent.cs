using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public struct HealthComponent : IComponent
{
    public int CurrentHealth;
    public int MaxHealth;
}
