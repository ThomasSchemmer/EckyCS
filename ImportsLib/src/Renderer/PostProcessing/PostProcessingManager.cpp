#include "PostProcessingManager.h"
#include "../../GameService/Game.h"
#include "../Passes/PostProcessingPass.h"
#include "../Passes/BasePass.h"
#include "../Renderer.h"

using namespace GameImports;

namespace PostProcessing
{
    void PostProcessingManager::Init()
    {
        RendererPtr = Game::Instance->RendererPtr;
        PixelShader = make_shared<class PixelShader>();
        PixelShader->Create();
        GeometryProvider = make_shared<EckyCS::FullScreenQuadProvider>();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VerticesBuffer);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VerticesBuffer);
        
        // copy vertices, UV and normals
        glBufferData(GL_ARRAY_BUFFER, GeometryProvider->GetVertexByteCount(), GeometryProvider->GetVertexArray(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 5));
        glEnableVertexAttribArray(2);
    }

    void PostProcessingManager::Update(RenderPassType Type)
    {
        // This needs an additional loop once we support more than one PP shader!
        if (!PixelShader->SupportsPass(Type))
            return;

        auto Pass = Game::Instance->RendererPtr->GetRenderPass<BasePass>();
        PixelShaderSettings Settings;
        Settings.ShadowMap = Pass->ColorTex;
        PixelShader->UpdateVars(Settings);
    }

    void PostProcessingManager::Render(RenderPassType Type)
    {
        // This needs an additional loop once we support more than one PP shader!
        if (!PixelShader->SupportsPass(Type))
            return;

        GPU_PROFILE(Game::GetGpuFrame(), "PostProcessing::Render", legit::Colors::greenSea);
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, PixelShader->Program, -1, "PostProcessing");
        PixelShader->Use(Type);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glPopDebugGroup();
    }

    void PostProcessingManager::CleanUp()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VerticesBuffer);
    }

}