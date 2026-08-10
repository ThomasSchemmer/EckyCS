#include "PixelShader.h"

#include "../Passes/RenderPass.h"
#include "../../Util/ShaderHelper.h"

void PixelShader::Create()
{
    const wchar_t* VertexShader = L"PIXEL_VERTEX_SHADER";
    const wchar_t* FragmentShader = L"PIXEL_FRAGMENT_SHADER";
    CreateInternal(VertexShader, FragmentShader);
}

bool PixelShader::SupportsPass(RenderPassType Type)
{
    return Type == RenderPassType::PostProcessingPass;
}

void PixelShader::UpdateVars(const shared_ptr<Camera>& Camera, const shared_ptr<Light>& Light) const
{
    BaseShader::UpdateVars(Camera, Light);
}

void PixelShader::UpdateVars(const PixelShaderSettings& Settings) const
{
    Util::ShaderHelper::SetUniformTexture("ShadowMap", Settings.ShadowMap, 0, Program);
}
