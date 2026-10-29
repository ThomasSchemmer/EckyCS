#include "DepthShader.h"

#include "../Passes/RenderPass.h"
#include "../../Util/ShaderHelper.h"

using namespace Util;

void DepthShader::Create()
{
    const wchar_t* ECSVertexShader = L"ECS_DEPTH_VERTEX_SHADER";
    const wchar_t* ECSFragmentShader = L"ECS_DEPTH_FRAGMENT_SHADER";
    CreateProgram(ECSVertexShader, ECSFragmentShader, "ECSDepthShaderProgram", ECSProgram);
    
    const wchar_t* VertexShader = L"DEPTH_VERTEX_SHADER";
    const wchar_t* FragmentShader = L"DEPTH_FRAGMENT_SHADER";
    CreateInternal(VertexShader, FragmentShader);
}

bool DepthShader::SupportsPass(RenderPassType Type)
{
    return Type == RenderPassType::DepthPrePass;
}


void DepthShader::Use(RenderPassType Type, void* UserData)
{
    BaseShader::Use(Type);
    bool* Enabled = static_cast<bool*>(UserData);
    if (Enabled && *Enabled)
    {
        glUseProgram(ECSProgram);
    }
}
