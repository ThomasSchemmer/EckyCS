#version 430

#include "COMMON_SHADER"

in vec4 BaseWorldPos;
in vec4 WorldNormals;
in vec2 UV;
in vec4 PosLightClip;
in float GrassNoise;

uniform vec3 GrassColor;

layout(binding = 0) uniform sampler2DArray FoliageTex;
layout(binding = 1) uniform sampler2D ShadowMap;

uniform vec3 LightDir;
uniform vec3 LightPos;
uniform vec2 LightClip;

out vec4 FragColor;
#include "SHADOW_SHADER"
#include "TERRAIN_CUBIC_SHADER"

const vec3 FlowerColors[] = vec3[](
    vec3(0.74, 0.68, 0.63), // Dusty beige rose
    vec3(0.62, 0.67, 0.61), // Muted sage green
    vec3(0.70, 0.66, 0.72), // Soft lavender gray
    vec3(0.66, 0.71, 0.74), // Pale blue-gray
    vec3(0.73, 0.70, 0.65)  // Warm linen tone
);

void main(){
    vec4 FoliageColor = texture(FoliageTex, vec3(UV, BaseWorldPos.w));
    if (FoliageColor.a < .5)
        discard;

    float tGrassNoise = map(GrassNoise, .0, 1.0, TerrainMinColor, TerrainMaxColor);
    vec3 Color = tGrassNoise * GrassColor;
    float FlowerNoise = fract(GrassNoise + 0.37);
    vec3 RndColor = FlowerColors[int(FlowerNoise * 5)].xyz;
    Color = BaseWorldPos.w > 0 ? RndColor : Color;

    float ShadowFactor = GetVarianceShadow();
    vec3 TempShadowColor = (ShadowColor * 2 + Color) / 3.0;
    Color = mix(TempShadowColor, Color, ShadowFactor);
    FragColor = vec4(Color, 1);
}