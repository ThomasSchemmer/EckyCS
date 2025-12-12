#include "DepthShader.h"

void DepthShader::Create()
{
    const wchar_t* VertexShader = L"DEPTH_VERTEX_SHADER";
    const wchar_t* FragmentShader = L"DEPTH_FRAGMENT_SHADER";
    CreateInternal(VertexShader, FragmentShader);
}
