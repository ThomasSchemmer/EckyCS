
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normals;
layout (location = 3) in vec3 offset;
layout (location = 4) in float type;

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
    WorldNormals = Transform * vec4(normals, 1);
    WorldPos = Transform * vec4(pos.xyz + offset, 1);
    gl_Position = Projection * View * WorldPos;
    Type = type;
    UV = uv;
}