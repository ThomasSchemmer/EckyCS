#version 430

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;

uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;

uniform sampler2D ShadowMap;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;

out vec4 FragColor;

#include "SHADOW_SHADER"
#include "LIGHT_SHADER"
#include "TERRAIN_COMMON_SHADER"
#include "COMMON_SHADER"

void main() {
    FragColor = vec4(0, 0, 1, 1);
}