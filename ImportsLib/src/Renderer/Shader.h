#pragma once
#include <memory>
#include <string>
#include <glew/include/GL/glew.h>
#include <glm/glm.hpp>

class Camera;
/**
 * Class changing the actual display of elements on the screen
 * Contains links to loaded OpenGL structs 
 */
using namespace std;
class Shader
{
    
public:
    unsigned int Program;
    unsigned int ContainerTex, CornTex;
    Shader();

    void Use();
    void UpdateVars(const shared_ptr<Camera>& Camera);

private:
    glm::mat4 Transform;
    
    static string LoadShader(const string& FilePath);
    static unsigned int CompileShader(const string& Code, GLenum Type);
    static unsigned int CreateProgram(unsigned int Vertex, unsigned int Fragment);
    static unsigned int CreateTexture(const string& FilePath, GLint Format);

    void SetUniform1f(const string& UniformName, float Value) const;
    void SetUniformM4(const string& UniformName, const glm::mat4& Value) const;
    void SetUniform3fv(const string& UniformName, const glm::vec3& Value) const;
    void SetUniformTexture(const string& UniformName, unsigned int TextureID, GLint Slot) const;

    string VertexShaderPath = "./src/Shaders/BaseVertexShader.vert";
    string FragmentShaderPath = "./src/Shaders/BaseFragmentShader.frag";
    
};
