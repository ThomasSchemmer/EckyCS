using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class Ability : GameplayAbility, ITargeted
{
    public AbilityType Type;

    public void OnAbilityHit(GameplayAbilityBehaviour Target)
    {
        OnTargetHit.ForEach(_ => _?.Invoke(Target));
    }

    protected bool TrySpawnProjectile(Projectile.Parameters Params, out Projectile Projectile)
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
        Projectile.Initialize(Params, this);

        return true;
    }

    public float GetCooldownCutoff()
    {
        return 1 - CurrentCooldown;
    }

    public ActionList<GameplayAbilityBehaviour> OnTargetHit = new();
}
