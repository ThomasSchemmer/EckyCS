#ifndef UNITY_SHADER_WORLDPOS
#define UNITY_SHADER_WORLDPOS

static float4 _CamForward = float4(0.61, -0.5, 0.61, 1);
static float4 _CamRight = float4(0.71, 0, -0.71, 1);
static float4 _CamUp = float4(0.35, 0.87, 0.35, 1);
static float _CamSize = 20;
static float2 _CamScreenSize = float2(_CamSize * 1.7777, _CamSize);
static float _CamAngle = 30;
static float deg2rad = PI / 180;

float3 GetWorldDepthPos(float2 ScreenUV, float Depth){
            
    // depth is negative, recalculate world pos according to it (and camera)
    float2 Target = (ScreenUV);
    float3 WorldDepthPos = 
        _WorldSpaceCameraPos.xyz + 
        // project to near plane
        -_CamForward.xyz * _ProjectionParams.y + 
        // raycast depth along camera angle 
        _CamForward.xyz * Depth * (_ProjectionParams.z - _ProjectionParams.y) + 
        // raycast to the sides according to uv
        Target.x * _CamRight.xyz * _CamScreenSize.x +
        Target.y * _CamUp.xyz * _CamScreenSize.y;
    return WorldDepthPos;
}

#endif