#version 430 

float cubicNoise(vec3 at);
float GetGrassNoise(vec4 WorldPos, float GrassScale, float GrassQuantize);

out vec4 FragColor;

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;

uniform vec3 BrushPos;
uniform uint BrushSize;
uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;
uniform sampler2D ShadowMap;

uniform vec3 TexColors[3];
uniform vec3 CliffColor;
uniform vec3 GrassColor;
uniform float GrassScale;
uniform float GrassQuantize;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;
uniform int ShowWireFrame;

const float BrushBorder = 0.25;

#include "SHADOW_SHADER"
#include "LIGHT_SHADER"
#include "TERRAIN_COMMON_SHADER"
#include "TERRAIN_CUBIC_SHADER"
#include "COMMON_SHADER"

// binding 0 and 1 are vertex and normal buffer in vertex shader. 2 is count buffer. 3 is height
// for more layout info see terrain mesh shader
layout(std430, binding = 3) buffer HeightBuffer {
    uint Values[];
} Heights;

#include "TERRAIN_TEXLOOKUP_SHADER"

vec3 GetBrushColor(void){
    ivec2 id = ivec2(UV.x * TexSize.x, UV.y * TexSize.y);
    
    // brush outline circle
    float d = distance(WorldPos.xz, BrushPos.xz);
    float TmpSize = BrushSize * 2;
    float a0 = smoothstep(TmpSize - BrushBorder, TmpSize, d);
    float a1 = smoothstep(TmpSize, TmpSize + BrushBorder, d);
    vec3 BrushColor = vec3(a0 - a1);

    // grid pattern based on world space
    ivec3 iWorld = ivec3(WorldPos) - ivec3(GlobalWorldPos);
    int Show = ((iWorld.x % 2) == 0) && ((iWorld.z % 2) == 0) ? 1 : 0;

    // visibility mask according to selection
    uint GlobalIndex = id.y * TexSize.x + id.x;
    uint Value = Heights.Values[GlobalIndex] >> LAYOUT_SELECTION;
    Value *= Show;
    
    return clamp(BrushColor + Value, 0, 1);
}

void main()
{
    vec3 BrushColor = GetBrushColor();
    float GrassNoise = GetGrassNoise(WorldPos, GrassScale, GrassQuantize);
    GrassNoise = map(GrassNoise, .0, 1.0, TerrainMinColor, TerrainMaxColor);
    vec3 Grass = GrassColor * GrassNoise;

    // sample all available textures and display only the highest one
    uint BaseIndex = GetBaseIndex(UV, TexSize);
    vec2 BaseID = UV * vec2(TexSize);
    uvec4 TexValues = GetTexValues(BaseIndex, TexSize);
    float Noise = cubicNoise(WorldPos.xyz / 2.0);
    float Tex0 = GetTexValueByLayout(BaseID, TexValues, LAYOUT_TEX0, Noise);
    float Tex1 = GetTexValueByLayout(BaseID, TexValues, LAYOUT_TEX1, Noise);
    float Tex2 = GetTexValueByLayout(BaseID, TexValues, LAYOUT_TEX2, Noise);

    float MaxValue = Tex1 >= Tex0 ? Tex1 : Tex0;
    uint Max = Tex1 >= Tex0 ? 1 : 0;
    Max = Tex2 >= MaxValue ? 2 : Max;
    MaxValue = Tex2 >= MaxValue ? Tex2 : MaxValue;
    Grass = mix(Grass, TexColors[Max], MaxValue);
    
    // make shadow lighter
    float LightFactor = GetLight();
    vec3 Cliff = CliffColor * map(LightFactor, 0, 1, .25, 1);
    
    float GrassFactor = abs(dot(vec3(0, 1, 0), WorldNormals.xyz));
    vec3 TexColor = GrassFactor * Grass + (1 - GrassFactor) * Cliff;

    float ShadowFactor = GetVarianceShadow();
    vec3 TempShadowColor = ShadowColor;//(ShadowColor * 2 + TexColor) / 3.0;
    vec3 Color = mix(TempShadowColor, TexColor, ShadowFactor);

    FragColor = vec4(BrushColor + Color, 1);
} 