using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Fireball", menuName = "ScriptableObjects/Abilities/Fireball", order = 0)]
public class Fireball : GameplayAbility
{
    public GameplayEffect OnHitEffect;

    private Projectile Projectile;

    public override void Commit()
    {
        base.Commit();
        Projectile.Parameters SpawnParams = new()
        {
            OnHitEffect = OnHitEffect,
            Position = AssignedToBehaviour.transform.position,
            Direction = AssignedToBehaviour.transform.forward,
            Type = AbilityType.Fireball,
            Speed = 5
        };
        GameplayAbilityLibrary.TrySpawnProjectile(this, SpawnParams, out Projectile);

        End();
    }
}
