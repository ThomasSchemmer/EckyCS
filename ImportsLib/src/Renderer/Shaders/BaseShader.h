#pragma once
#include <memory>
#include <string>
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>

class Light;
class Camera;
/**
 * Class changing the actual display of elements on the screen
 * Contains links to loaded OpenGL structs
 * Displays a very basic, phong-lit material 
 */
using namespace std;
class BaseShader
{
    
public:
    unsigned int Program;
    BaseShader() = default;
    virtual ~BaseShader() = default;

    virtual void Create();
    void Use() const;
    void UpdateVars(const shared_ptr<Camera>& Camera, const shared_ptr<Light>& Light) const;
    void CleanUp() const;

protected:
    glm::mat4 Transform;
    void CreateInternal(const wchar_t* VShader, const wchar_t* FShader);
};
