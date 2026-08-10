#pragma once
#include <memory>
#include <string>

#include <GL/glew.h>

#include "Shader.h"
#include "glm/matrix.hpp"

class Light;
class Camera;
/**
 * Class changing the actual display of elements on the screen
 * Contains links to loaded OpenGL structs
 * Displays a very basic, phong-lit material 
 */
using namespace std;
class BaseShader : public Shader
{
    
public:
    BaseShader() = default;

    virtual void Create() override;
    virtual void UpdateVars(const shared_ptr<Camera>& Camera, const shared_ptr<Light>& Light) const;
    virtual bool SupportsPass(RenderPassType Type) override;
    virtual void Use(RenderPassType Type) override;
    virtual void CleanUp() const override;
    

protected:
    glm::mat4 Transform;
    void CreateInternal(const wchar_t* VShader, const wchar_t* FShader);
};
