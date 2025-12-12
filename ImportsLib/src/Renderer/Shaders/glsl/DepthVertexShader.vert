
#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 Transform;
uniform mat4 View;
uniform mat4 Projection; 

void main(){
    vec4 WorldPos = Transform * vec4(pos.xyz, 1);
    gl_Position = Projection * View * WorldPos;
}