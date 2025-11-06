#version 330 core
out vec4 FragColor;

in vec4 Color;
in vec2 UV;

uniform sampler2D ContainerTex;
uniform sampler2D CornTex;

void main()
{
    vec4 ContainerColor = texture(ContainerTex, UV);
    vec4 CornColor = texture(CornTex, UV);
    vec4 TexColor = UV.x > .5 ? CornColor : ContainerColor;
    FragColor = mix(TexColor, Color, .5);
} 