
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normals;

uniform mat4 Transform;
uniform mat4 View;
uniform mat4 Projection;
out vec4 WorldNormals;
out vec4 WorldPos;
out vec2 UV;

void main()
{
    WorldNormals = vec4(normals, 1); //Transform * 
    WorldPos = vec4(pos.xyz, 1);
    UV = uv;
    gl_Position = WorldPos; //Projection * View * 
}