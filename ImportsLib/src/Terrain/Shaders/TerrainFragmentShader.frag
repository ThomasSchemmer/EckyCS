#version 330 core
out vec4 FragColor;

in vec4 WorldPos;

uniform vec3 BrushPos;

void main()
{
    float d = 1 - clamp(distance(BrushPos, WorldPos.xyz), 0, 1);
    FragColor = vec4(d, d, d, 1);
} 