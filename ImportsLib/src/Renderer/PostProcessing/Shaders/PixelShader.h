#pragma once
#include "../../Shaders/BaseShader.h"
#include "../../Shaders/Shader.h"


/**
 * Helper struct to contain all values necessary in the Pixel PP Shader
 */
class PixelShaderSettings
{
public:
    GLuint ShadowMap;
};

/**
 * Shader that renders the previously rendered scene onto a fullscreen
 * quad, creating postprocessing effects
 */
class PixelShader : public BaseShader
{
public:
    void Create() override;
    bool SupportsPass(RenderPassType Type) override;
    /** Gets called from the renderer, handles light/shadow */
    void UpdateVars(const shared_ptr<Camera>& Camera, const shared_ptr<Light>& Light) const override;
    /** Gets called from the PP Manager, handles texture parsing */ 
    void UpdateVars(const PixelShaderSettings& Settings) const;
};
