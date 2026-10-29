#version 330 core

#include "COMMON_SHADER"

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normals;
layout (location = 3) in vec3 offset;
layout (location = 4) in vec4 quat;

uniform mat4 Transform; 
uniform mat4 View; 
uniform mat4 Projection; 
uniform vec3 SunPos;
out float Type;
out vec2 UV;
out vec4 WorldNormals;
out vec4 WorldPos;

void main()
{
    vec3 tempPos = rotate(pos.xyz, quat);
    WorldNormals = Transform * vec4(normals, 1);
    WorldPos = Transform * vec4(tempPos + offset, 1);

    gl_Position = Projection * View * WorldPos;
    Type = 0;//type;
    UV = uv;
}