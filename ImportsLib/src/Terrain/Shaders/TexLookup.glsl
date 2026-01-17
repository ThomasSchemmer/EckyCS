
uint GetBaseIndex(vec2 UV, ivec2 TexSize){
    ivec2 baseId = ivec2(UV * vec2(TexSize));
    uint BaseIndex = uint(baseId.y * TexSize.x + baseId.x);
    return BaseIndex;
}

uvec4 GetTexValues(uint BaseIndex, ivec2 TexSize){
    uint d00 = Heights.Values[BaseIndex + 0];
    uint d10 = Heights.Values[BaseIndex + 1];
    uint d01 = Heights.Values[BaseIndex + TexSize.x + 0];
    uint d11 = Heights.Values[BaseIndex + TexSize.x + 1];
    return uvec4(d00, d10, d01, d11);
}

/** Samples the provided heightmap values with noise based on world position */
float GetTexValueByLayout(vec2 BaseID, uvec4 TexValues, uint TargetLayout, float Noise){
    uint TargetMask = 1u << TargetLayout;

    uint d00 = TexValues.x & TargetMask;
    uint d10 = TexValues.y & TargetMask;
    uint d01 = TexValues.z & TargetMask;
    uint d11 = TexValues.w & TargetMask;

    vec2 f = fract(BaseID);
    float d0 = mix(float(d00), float(d10), f.x);
    float d1 = mix(float(d01), float(d11), f.x);
    float ValueSmooth = mix(d0, d1, f.y) / float(TargetMask);
    
    // make a smooth transition 
    ValueSmooth *= Noise;
    // cutoff to have a better transition
    float CutOff = .25;
    float Quantize = 8;
    ValueSmooth = ValueSmooth > CutOff ? map(ValueSmooth, CutOff, 1, 0.65, 1) : 0;
    ValueSmooth = int(ValueSmooth * Quantize) / Quantize;
    return ValueSmooth;
}