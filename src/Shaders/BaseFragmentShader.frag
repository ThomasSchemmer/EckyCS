#version 330 core
out vec4 FragColor;

in vec4 WorldNormals;
in vec4 WorldPos;
in float Type;
in vec2 UV;

uniform sampler2D ContainerTex;
uniform sampler2D CornTex;
uniform vec3 SunPos;
uniform vec3 CamPos;

float GetLight(){
    float ambi = 0.1;
    vec3 Norm = normalize(WorldNormals.xyz);
    vec3 lightDir = normalize(SunPos - WorldPos.xyz);
    float diff = max(dot(Norm, lightDir), 0.0);
    
    vec3 viewDir = normalize(CamPos - WorldPos.xyz);
    vec3 reflectDir = reflect(-lightDir, Norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    return spec + diff + ambi;
}

void main()
{
    float a = GetLight();
    vec4 BaseColor = Type > 4 ? vec4(1, 0, 0, 1) : vec4(1);
    BaseColor.xyz *= a;
    FragColor = BaseColor;
    return;
    //vec4 ContainerColor = texture(ContainerTex, UV);
    //vec4 CornColor = texture(CornTex, UV);
    //vec4 TexColor = UV.x > .5 ? CornColor : ContainerColor;
    //FragColor = mix(TexColor, Color, .5);
} 