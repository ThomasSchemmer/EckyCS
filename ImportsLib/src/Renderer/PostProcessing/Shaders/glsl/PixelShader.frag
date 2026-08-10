#version 330 core
out vec4 FragColor;

in vec4 WorldNormals;
in vec4 WorldPos;
in vec2 UV;
uniform sampler2D ShadowMap;

void main()
{
    vec4 Img = texture(ShadowMap, UV);
    FragColor = vec4(Img.xyz, 1);
    return;
} 