#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Util/Projector.h"

class Camera;

class Light : public Projector
{
public:
    glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);

    Light(glm::vec3 InPosition, glm::vec3 InEuler, glm::vec3 InColor, GLFWwindow* Window, const std::shared_ptr<Camera>& Cam);

    void OnDrawGizmos();
    void Update(float delta);

private:
    std::shared_ptr<Camera> CameraPtr;

    void CalculateFrustum();
};
