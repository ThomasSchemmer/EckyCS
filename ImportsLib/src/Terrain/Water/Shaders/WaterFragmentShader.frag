#version 430

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;

uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;
uniform vec2 ScreenSize;
uniform float Time;

uniform sampler2D DepthTex;
uniform sampler2D CausticTex;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;

out vec4 FragColor;

#include "LIGHT_SHADER"
#include "TERRAIN_COMMON_SHADER"
#include "COMMON_SHADER"
#include "TERRAIN_NOISE_SHADER"

const float DepthThreshold = 0.0005f;
const vec3 DeepWaterColor = vec3(15,94,156) / 255.0;
const vec3 ShallowWaterColor = vec3(28,163,236) / 255.0;
const vec3 CausticColor = vec3(200, 200, 200) / 255.0;
const vec3 FoamColor = vec3(240, 240, 240) / 255.0;

const vec2 CausticScale = vec2(64, 64);
const float CausticEffect = 0.01;
const float CausticBlend = 0.2;

const float PixScale = 4;

void main() {
    float SelfDepth = gl_FragCoord.z;
    vec2 ScreenUv = gl_FragCoord.xy / ScreenSize;
    float RefDepth = texture(DepthTex, ScreenUv).r;
    
    float RawDepthDiff = RefDepth - SelfDepth;
    float DepthDiff = 0.0;
    float FoamFactor = 0.0;
    if (RefDepth < 0.95 && RawDepthDiff > 0.01)
    {
        DepthDiff = clamp(RawDepthDiff * 25, 0.0, 1.0);
        FoamFactor = 1.0 - smoothstep(0.0, 0.2, DepthDiff * .25);
    }
    
    vec3 WaterColor = mix(ShallowWaterColor, DeepWaterColor, DepthDiff);
    
    vec2 pixWorldPos = floor(WorldPos.xz * PixScale) / PixScale;
    vec2 UV = pixWorldPos / CausticScale;
    UV += valueNoise2D(pixWorldPos + vec2(Time)) * CausticEffect;
    float CausticFactor = texture(CausticTex, UV).a * CausticBlend;
    
    float FoamCausticFactor = texture(CausticTex, UV * 3).a;
    FoamFactor *= FoamCausticFactor * 2;
    
    WaterColor = mix(WaterColor, CausticColor, CausticFactor);
    WaterColor = mix(WaterColor, FoamColor, FoamFactor);
    FragColor = vec4(WaterColor, DepthDiff > 0.01);

}