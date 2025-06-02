using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
[RequireComponent(typeof(MeshCollider))]
public class Projectile : MonoBehaviour
{
    public GameplayEffect OnHitEffect;

    private Vector3 Dir;
    private float Speed = 1;
    private Ability OwningAbility;
        
    void OnCollisionEnter(Collision Collision)
    {
        if (!Collision.gameObject.TryGetComponent<GameplayAbilityBehaviour>(out var Behaviour))
            return;

        OwningAbility.OnAbilityHit(Behaviour);
    }

    void Update()
    {
        transform.position += Dir * Time.deltaTime * Speed;
    }

    public void Initialize(Parameters Params, Ability Ability)
    {
        Dir = Params.Direction;
        Speed = Params.Speed;
        OnHitEffect = Params.OnHitEffect;
        OwningAbility = Ability;
        OwningAbility.OnTargetHit.Add(OnProjectileHit);
        transform.SetPositionAndRotation(
            Params.Position,
            Quaternion.LookRotation(Dir, Vector3.up)
        );
        gameObject.layer = LayerMask.NameToLayer("Ability");
    }

    private void OnProjectileHit(GameplayAbilityBehaviour Target)
    {
        if (!Game.TryGetService(out GameplayAbilitySystem GAS))
            return;

        GAS.TryApplyEffectTo(Target, OnHitEffect);
    }

    private void OnDestroy()
    {
        if (OwningAbility == null)
            return;

        OwningAbility.OnTargetHit.Remove(OnProjectileHit);
    }

    public class Parameters
    {
        public Vector3 Position;
        public Vector3 Direction;
        public GameplayEffect OnHitEffect;
        public AbilityType Type;
        public float Speed;
    }
}
