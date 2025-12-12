#pragma once
#include "RenderPass.h"

/**
 * Handler for all shadow generation calculations
 * Uses variance shadow maps, so we render first into a depth tex
 * and then blur.
 * Todo: Make depth SSBO
 */
class ShadowPass : public RenderPass
{
public:
    void Create(GLFWwindow* Window) override;
    void OnAfterRender() override;
    void CleanUp() const override;

private:
    GLuint BlurTexCompute = 0;
    GLuint TempOut = 0;

    const wchar_t* BlurTexComputeID = L"BLUR_TEX_COMPUTE_SHADER";
};