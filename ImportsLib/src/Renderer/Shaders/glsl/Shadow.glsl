
const float MinBias = 0.0005;
const float MaxBias = 0.002;
const vec3 ShadowColor = vec3(0.01, 0.01, 0.03);

float LinearStep(float Value, float Low, float High){
    return clamp((Value - Low) / (High - Low), 0, 1);
}

float ChebyshevUpperBound(vec2 Moments, float t){
    float p = step(t, Moments.x);
    float Variance = Moments.y - (Moments.x * Moments.x);
    Variance = max(Variance, 0.00002);
    
    float d = t - Moments.x;
    float PMax = Variance / (Variance + d * d);
    // fight light bleeding
    PMax = LinearStep(PMax, 0.2, 1);
    return clamp(max(p, PMax), 0, 1);
}

float GetVarianceShadow(void){
    vec3 NDC = PosLightClip.xyz / PosLightClip.w;
    NDC = NDC / 2.0 + 0.5;
    vec2 UV = NDC.xy;
    vec2 Moments = texture(ShadowMap, UV).xy;
    return ChebyshevUpperBound(Moments, NDC.z);
}

float GetShadow(void){
    // check if in the ShadowMap the depth is lower than the current one -> we are in shadow
    vec3 NDC = PosLightClip.xyz / PosLightClip.w;
    NDC = NDC / 2.0 + 0.5;
    vec2 UV = NDC.xy;
    float Depth = texture(ShadowMap, UV).r;
    float CurrentDepth = NDC.z;
    float Bias = max(MaxBias * (1.0 - dot(WorldNormals.xyz, LightDir)), MinBias);
    float Shadow = (CurrentDepth < Depth + Bias) ? 1 : 0;
    Shadow = NDC.z > 1 ? 0 : Shadow;
    return Shadow;
}