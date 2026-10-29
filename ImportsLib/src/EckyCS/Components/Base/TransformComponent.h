#pragma once
#include "../Component.h"
#include <glm/gtc/quaternion.hpp>

namespace EckyCS
{
    struct TransformComponent : Component
    {
    public:
        float PosX;
        float PosY;
        float PosZ;
        float QuatX;
        float QuatY;
        float QuatZ;
        float QuatW;

        void SetRotation(float X, float Y, float Z)
        {
            glm::quat quat = glm::quat(glm::vec3(X, Y, Z));
            QuatX = quat.x;
            QuatY = quat.y;
            QuatZ = quat.z;
            QuatW = quat.w;
        }
    
    };
}
