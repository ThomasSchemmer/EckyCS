
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 3) in vec3 offset;

uniform mat4 Transform; 
uniform mat4 View; 
uniform mat4 Projection; 
out vec4 WorldPos;

void main()
{
    WorldPos = Transform * vec4(pos.xyz + offset, 1);
    gl_Position = Projection * View * WorldPos;
}