
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;

uniform mat4 Transform; 
uniform mat4 View; 
uniform mat4 Projection; 
out vec3 Color;
out vec2 UV;

void main()
{
    gl_Position = Projection * View * Transform * vec4(pos, 1);
    Color = color;
    UV = uv;
}