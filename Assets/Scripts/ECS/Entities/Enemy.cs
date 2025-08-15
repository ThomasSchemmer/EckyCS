using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponentType(
    //typeof(HealthComponent), 
    typeof(TransformComponent),
    typeof(RenderComponent)
)]  
public class Enemy : Entity
{
}
