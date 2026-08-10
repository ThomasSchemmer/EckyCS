#pragma once
#include <memory>

#include <glew/include/GL/glew.h>
#include <GLFW/include/GLFW/glfw3.h>
#include "../../EckyCS/Systems/Rendering/FullScreenQuadProvider.h"
#include "Shaders/PixelShader.h"

class Renderer;
enum class RenderPassType : unsigned char;

namespace PostProcessing
{
    /** Manager class that handles switching to the correct PP shaders, loading data etc */
    class PostProcessingManager : public enable_shared_from_this<PostProcessingManager>
    {
    public:
        PostProcessingManager() = default;
        ~PostProcessingManager() = default;

        void Init();
        void Update(RenderPassType Type);
        void Render(RenderPassType Type);
        void CleanUp();

        std::shared_ptr<BaseShader> GetShader()
        {
            return PixelShader;
        }

    private:
        std::shared_ptr<PixelShader> PixelShader;
        std::shared_ptr<Renderer> RendererPtr;
        std::shared_ptr<EckyCS::FullScreenQuadProvider> GeometryProvider;

        GLuint VAO;
        GLuint VerticesBuffer = 0;
    };
}
