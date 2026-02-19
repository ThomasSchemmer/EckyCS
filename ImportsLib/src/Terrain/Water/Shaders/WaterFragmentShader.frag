#version 430

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;

uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;

uniform sampler2D DepthTex;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;
uniform vec2 DepthThreshold;

out vec4 FragColor;

//#include "SHADOW_SHADER"
#include "LIGHT_SHADER"
#include "TERRAIN_COMMON_SHADER"
#include "COMMON_SHADER"

void main() {
    float SelfDepth = gl_FragCoord.z;
    vec2 uv = gl_FragCoord.xy / vec2(1920, 1200);
    float RefDepth = texture(DepthTex, uv).r;
    //float DepthDiff = smoothstep(DepthThreshold.x, DepthThreshold.y, abs(RefDepth - SelfDepth));
    float DepthDiff = (RefDepth - SelfDepth);
    FragColor = vec4(DepthDiff * 100, 0, 0, 1);
}