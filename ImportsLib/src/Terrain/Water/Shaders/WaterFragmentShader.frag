#version 430

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;

uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;
uniform vec2 ScreenSize;

uniform sampler2D DepthTex;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;

out vec4 FragColor;

#include "LIGHT_SHADER"
#include "TERRAIN_COMMON_SHADER"
#include "COMMON_SHADER"

const float DepthThreshold = 0.0005f;
const vec3 DeepWaterColor = vec3(15,94,156) / 255.0;
const vec3 ShallowWaterColor = vec3(28,163,236) / 255.0;

void main() {
    float SelfDepth = gl_FragCoord.z;
    vec2 uv = gl_FragCoord.xy / ScreenSize;
    float RefDepth = texture(DepthTex, uv).r;
    RefDepth = RefDepth > 0.95 ? SelfDepth : RefDepth;
    float DepthDiff = clamp((RefDepth - SelfDepth) * 25, 0, 1);
    vec3 WaterColor = mix(ShallowWaterColor, DeepWaterColor, DepthDiff);
    float FoamFactor = 1 - smoothstep(0.0, 0.2, DepthDiff);
    
    FragColor = vec4(WaterColor + vec3(FoamFactor), 1);
}