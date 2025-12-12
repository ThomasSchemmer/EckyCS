#version 330 core
out vec4 FragColor;

in vec4 WorldNormals;
in vec4 WorldPos;
in float Type;
in vec2 UV;

uniform vec3 LightPos;
uniform vec3 LightDir;
uniform vec3 CamPos;

float GetLight(void){
    float ambi = 0.1;
    vec3 Norm = normalize(WorldNormals.xyz);
    float diff = max(dot(Norm, LightDir), 0.0);
    
    vec3 viewDir = normalize(CamPos - WorldPos.xyz);
    vec3 reflectDir = reflect(-LightDir, Norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    return spec + diff + ambi;
}

void main()
{
    float a = GetLight();
    FragColor = vec4(WorldNormals.xyz * a, 1);
    return;
} 