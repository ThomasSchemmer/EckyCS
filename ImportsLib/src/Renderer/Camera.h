#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
    glm::mat4 Projection, View;
    glm::vec3 Position;

    void Update();
    Camera(const std::function<int(int)>& InputCallback);

private:
    std::function<int(int)> InputCallback;
    float MoveSpeed;
    
    void ProcessInput();
};
