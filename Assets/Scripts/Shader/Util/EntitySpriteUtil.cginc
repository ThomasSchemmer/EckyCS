#ifndef UNITY_SHADER_ENTITYSPRITE
#define UNITY_SHADER_ENTITYSPRITE

#include "WorldPos.cginc" 
#include "GpuPrinter.cginc" 
/**
 * Contains util functions used in the EntitySpriteShader passes
 */

 
CBUFFER_START(UnityPerMaterial)
    sampler2D _MainTex;
    float4 _Size;
    float4 _CamOffset;
    float4 _Scale;
CBUFFER_END


struct v2f
{
    float2 uv : TEXCOORD0;
    float4 vertex : SV_POSITION;
    float3 midPoint : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};
           

v2f GetV2F(float3 WorldPos, float3 MidPoint, float2 uv){
    v2f o;
    o.vertex = TransformWorldToHClip(WorldPos);
    o.uv = uv;
    o.midPoint = MidPoint;
    o.worldPos = WorldPos;
    return o;
}

float4 GetColor(uint Type, uint Amount, v2f i){

    float2 pxCoord = WorldToPixel(i.worldPos);
    float c1 = DrawNumberAtWorldPos(pxCoord, i.midPoint, Amount, 4, 0); 
    uint x = Type % (uint)_Size.z;
    uint y = Type / (uint)_Size.w;
    // cutout smaller picture from combined one
    float2 uv = (float2(x, y) * _Size.xy + i.uv * _Size.xy) / (_Size.xy * _Size.zw);
    float4 col = tex2D(_MainTex, uv);
    return float4(max(c1.xxx, col.xyz), col.a + c1);
}


#endif