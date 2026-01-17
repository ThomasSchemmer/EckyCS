#version 430

#include "COMMON_SHADER"

in vec4 BaseWorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;
in float GrassNoise;

uniform vec3 GrassColor;
uniform sampler2D GrassTex;

uniform sampler2D ShadowMap;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;

out vec4 FragColor;
#include "SHADOW_SHADER"
#include "TERRAIN_CUBIC_SHADER"

void main(){
    vec4 Grass = texture(GrassTex, UV); 
    if (Grass.a < .5)
        discard;

    float tGrassNoise = map(GrassNoise, .0, 1.0, TerrainMinColor, TerrainMaxColor);
    vec3 Color = tGrassNoise * GrassColor;

    float ShadowFactor = GetVarianceShadow();
    vec3 TempShadowColor = (ShadowColor * 2 + Color) / 3.0;
    Color = mix(TempShadowColor, Color, ShadowFactor);
    FragColor = vec4(Color, 1);
}