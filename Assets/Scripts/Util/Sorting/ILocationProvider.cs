using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface ILocationProvider
{
    public List<EntityID> GetAllAt(Vector3 Position, float Range);
}
