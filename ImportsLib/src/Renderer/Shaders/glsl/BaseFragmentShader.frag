#version 330 core
out vec4 FragColor;

in vec4 WorldNormals;
in vec4 WorldPos;
in float Type;
in vec2 UV;

uniform vec3 LightPos;
uniform vec3 LightDir;
uniform vec3 CamPos;

#include "LIGHT_SHADER"

void main()
{
    float a = GetLight();
    FragColor = vec4(WorldNormals.y, 0, 0, 1);
    return;
} 