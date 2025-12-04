#version 430 

float cubicNoise(vec3 at);
#include "TERRAIN_CUBIC_SHADER"

out vec4 FragColor;

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;

uniform vec3 BrushPos;
uniform uint BrushSize;
uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;
uniform ivec2 WorldSize;
uniform sampler2D ResultTex;

uniform vec3 GrassColor;
uniform vec3 DirtColor;
uniform float GrassScale;
uniform float GrassQuantize;
const vec3 LightDir = vec3(-0.41, -0.86, -0.28);

// 0 and 1 are vertex and normal buffer in vertex shader. 2 is count buffer. 3 is height
layout(std430, binding = 3) buffer HeightBuffer {
    uint Values[];
} Heights;

layout(std430, binding = 4) buffer SelectionBuffer {
    uint Entries[];
} Selection;

const float BrushBorder = 0.25;

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
    uint Value = Selection.Entries[GlobalIndex];
    Value *= Show;
    
    return clamp(BrushColor + Value, 0, 1);
}

float GetLight(void){
    float _Quantize = 15;
    float nl = max(0, dot(normalize(WorldNormals.xyz), LightDir));
    nl = int(nl * _Quantize) / _Quantize;
    return nl;
}


void main()
{
    vec3 BrushColor = GetBrushColor();
    float GrassNoise = abs(cubicNoise(WorldPos.xyz * GrassScale));
    GrassNoise += abs(cubicNoise(-WorldPos.xyz * GrassScale * 2)) * 0.5;
    GrassNoise += 0.2;
    GrassNoise = int(GrassNoise * GrassQuantize) / GrassQuantize;
    GrassNoise = clamp(GrassNoise, 0, 1);
    float nl = GetLight();
    vec3 Grass = GrassColor * GrassNoise;
    vec3 Dirt = DirtColor;
    float GrassFactor = abs(dot(vec3(0, 1, 0), WorldNormals.xyz));
    vec3 TexColor = GrassFactor * Grass + (1 - GrassFactor) * Dirt;
    TexColor *= nl;
    
    FragColor = vec4(BrushColor + TexColor, 1);


    
} 