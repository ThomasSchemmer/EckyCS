#include "Transform.h"

glm::vec4 Transform::WorldForward = glm::vec4(1, 0, 0, 0);
glm::vec4 Transform::WorldUp = glm::vec4(0, 1, 0, 0);

glm::vec3 Transform::GetForward() const
{
    glm::vec3 Forward;
    Forward.x = cos(EulerAngles.x) * cos(EulerAngles.y);
    Forward.y = sin(EulerAngles.x);
    Forward.z = cos(EulerAngles.x) * sin(EulerAngles.y);
    return -normalize(Forward);
}

glm::vec3 Transform::GetRight() const
{
    return normalize(cross(GetForward(), xyz(WorldUp)));
}

glm::vec3 Transform::GetUp() const
{
    return normalize(-cross(GetForward(), GetRight()));
}