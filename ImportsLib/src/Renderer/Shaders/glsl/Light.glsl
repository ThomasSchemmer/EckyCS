
const float _Quantize = 15;

float GetSpecLight(void){
    //float ambi = 0.1;
    //vec3 Norm = normalize(WorldNormals.xyz);
    //float diff = max(dot(Norm, LightDir), 0.0);
    //
    //vec3 viewDir = normalize(CamPos - WorldPos.xyz);
    //vec3 reflectDir = reflect(-LightDir, Norm);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    //
    //return spec + diff + ambi;
    return 0;
}

float GetLight(void){
    float nl = max(0, dot(normalize(WorldNormals.xyz), -LightDir));
    nl = int(nl * _Quantize) / _Quantize;
    return nl;
}