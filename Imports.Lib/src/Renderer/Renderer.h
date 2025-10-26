#pragma once
#include <renderdoc/renderdoc_app.h>

class Shader;

/**
 * Class enabling displaying objects on the screen
 * Contains shaders to change visuals and helpful debug tools 
 */
class Renderer
{
public:
    unsigned int VBO, VAO;

    ~Renderer();
    void Init();
    void InitRenderDoc();
    void Render() const;
    void HandleCaptureStart() const;
    void HandleCaptureStop() const;

private:
    Shader* ShaderPtr = nullptr;
    RENDERDOC_API_1_1_2* RDocAPI = nullptr;
    
    void CreateVertexBuffer();
#ifdef _WIN32
    void LoadRenderDocWindows();
    void UnloadRenderDocWindows();
#endif
    
    float Vertices[9] = {
        -0.5f, -0.5f, 0.0f, // Bottom-left
         0.5f, -0.5f, 0.0f, // Bottom-right
         0.0f,  0.5f, 0.0f  // Top
    };

    char RenderDocPath[42] = "./Imports.Lib/ext/renderdoc/renderdoc.dll";
};
