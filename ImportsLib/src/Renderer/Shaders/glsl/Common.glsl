

float map(float v, float min, float max, float nMin, float nMax){
    return (v - min) / (max - min) * (nMax - nMin) + nMin;
}

vec3 rotate(vec3 p, vec4 quat){
    return p.xyz + 2.0 * cross(quat.xyz, cross(quat.xyz, p.xyz) + quat.w * p.xyz);
}