#pragma once
#include <memory>
#include <string>
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>

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
    unsigned int ContainerTex, CornTex;
    BaseShader();
    ~BaseShader();

    void Use();
    void UpdateVars(const shared_ptr<Camera>& Camera);

private:
    glm::mat4 Transform;    
    const wchar_t* VertexShader = L"BASE_VERTEX_SHADER";
    const wchar_t* FragmentShader = L"BASE_FRAGMENT_SHADER";
    
};
