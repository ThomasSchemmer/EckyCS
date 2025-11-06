#pragma once
#include <memory>
#include <renderdoc/renderdoc_app.h>

#include "Camera.h"


class Shader;

/**
 * Class enabling displaying objects on the screen
 * Contains shaders to change visuals and helpful debug tools 
 */
class Renderer
{
public:
    unsigned int VBO, VAO;

    Renderer() = default;
    ~Renderer();
    void Init(const std::function<int(int)>& InputCallback);
    void InitRenderDoc();
    void Update();
    void Render() const;
    void HandleCaptureStart() const;
    void HandleCaptureStop() const;

private:
    Shader* ShaderPtr = nullptr;
    RENDERDOC_API_1_1_2* RDocAPI = nullptr;
    std::shared_ptr<Camera> Camera;
    
    void CreateVertexBuffer();
#ifdef _WIN32
    void LoadRenderDocWindows();
    void UnloadRenderDocWindows() const;
#endif
    
    float Vertices[24] = {
        // position          // color    // uv
        -.5f, -.5f, .0f,     1, 0, 0,    0,    0,    // Bottom-left
         .5f, -.5f, .0f,     0, 1, 0,    1,    0,    // Bottom-right
         .0f,  .5f, .0f,     0, 0, 1,    0.5f, 1     // Top
    };

    char RenderDocPath[42] = "./Imports.Lib/ext/renderdoc/renderdoc.dll";
};
