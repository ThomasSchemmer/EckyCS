#version 430

float cubicNoise(vec3 at);
float GetGrassNoise(vec4 WorldPos, float GrassScale, float GrassQuantize);
#include "TERRAIN_CUBIC_SHADER"

in vec4 BaseWorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;

// calculating grass color, depending on underlying terrain
uniform sampler2D GrassTex;
uniform vec3 GrassColor;
uniform float GrassScale;
uniform float GrassQuantize;

uniform sampler2D ShadowMap;
uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;

out vec4 FragColor;


#include "SHADOW_SHADER"

void main(){
    vec4 Grass = texture(GrassTex, UV); 
    if (Grass.a < .5)
        discard;

    float GrassNoise = GetGrassNoise(BaseWorldPos, GrassScale, GrassQuantize);
    vec3 Color = GrassNoise * GrassColor;

    float ShadowFactor = GetVarianceShadow();
    if (ShadowFactor < .75)
        discard;
    Color = mix(ShadowColor, Color, ShadowFactor);
    FragColor = vec4(Color, 1);
}