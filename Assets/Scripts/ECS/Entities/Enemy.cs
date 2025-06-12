using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponentType(
    //typeof(HealthComponent), 
    typeof(TransformComponent),
    typeof(SpriteComponent)
)]  
public class Enemy : Entity
{
}
