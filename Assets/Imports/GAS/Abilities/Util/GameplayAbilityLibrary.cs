using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class GameplayAbilityLibrary
{
    public static bool TrySpawnProjectile(GameplayAbility Parent, Projectile.Parameters Params, out Projectile Projectile)
    {
        Projectile = null;
        if (!Game.TryGetService(out MeshFactory MeshFactory))
            return false;

        Mesh Mesh = MeshFactory.GetAbilityMesh(Params.Type);
        GameObject ProjectileGO = new GameObject(Params.Type.ToString());
        //also adds required comps
        Projectile = ProjectileGO.AddComponent<Projectile>();
        ProjectileGO.GetComponent<MeshCollider>().sharedMesh = Mesh;
        ProjectileGO.GetComponent<MeshFilter>().mesh = Mesh;
        Projectile.Initialize(Params, Parent);

        return true;
    }


}
