//todo: get actually good and efficient noise!

const float TerrainMinColor = 0.4;
const float TerrainMaxColor = 0.9;

//https://github.com/jobtalle/CubicNoise/blob/master/glsl/cubicNoise.glsl
float random(vec3 x) {
    return fract(sin(x.x + x.y * 57.0 + x.z * 113.0) * 43758.5453);
}

float interpolate(float a, float b, float c, float d, float x) {
    float p = (d - c) - (a - b);

    return x * (x * (x * p + ((a - b) - p)) + (c - a)) + b;
}

float sampleX(vec3 at) {
    float floored = floor(at.x);

    return interpolate(
        random(vec3(floored - 1.0, at.yz)),
        random(vec3(floored, at.yz)),
        random(vec3(floored + 1.0, at.yz)),
        random(vec3(floored + 2.0, at.yz)),
        fract(at.x)) * 0.5 + 0.25;
}

float sampleY(vec3 at) {
    float floored = floor(at.y);

    return interpolate(
        sampleX(vec3(at.x, floored - 1.0, at.z)),
        sampleX(vec3(at.x, floored, at.z)),
        sampleX(vec3(at.x, floored + 1.0, at.z)),
        sampleX(vec3(at.x, floored + 2.0, at.z)),
        fract(at.y));
}

/** Bad performance due to multiple sin calls */
float cubicNoise(vec3 at) {
    float floored = floor(at.z);

    return interpolate(
        sampleY(vec3(at.xy, floored - 1.0)),
        sampleY(vec3(at.xy, floored)),
        sampleY(vec3(at.xy, floored + 1.0)),
        sampleY(vec3(at.xy, floored + 2.0)),
        fract(at.z));
}

vec3 hashv3(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.xxy + p.yzz) * p.zyx);
}

float hash3(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float hash2(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

vec2 hashv2(vec2 p)
{
    uvec2 q = floatBitsToUint(p);
    q *= uvec2(1597334677u, 3812015801u);
    q ^= q >> 16;
    return vec2(q) * (1.0 / 4294967296.0);
}

/** Bell curve distribution, values at end points basically impossible without flattening! */
float simplexNoise(vec3 p) {
    const float F3 = 1.0 / 3.0;
    const float G3 = 1.0 / 6.0;

    vec3 i = floor(p + dot(p, vec3(F3)));
    vec3 x0 = p - i + dot(i, vec3(G3));

    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g, l.zxy);
    vec3 i2 = max(g, l.zxy);

    vec3 x1 = x0 - i1 + G3;
    vec3 x2 = x0 - i2 + 2.0 * G3;
    vec3 x3 = x0 - 1.0 + 3.0 * G3;

    vec4 w = max(0.6 - vec4(
        dot(x0,x0),
        dot(x1,x1),
        dot(x2,x2),
        dot(x3,x3)
    ), 0.0);

    vec4 w4 = w * w * w * w;

    float Result = dot(w4, vec4(
        dot(hashv3(i), x0),
        dot(hashv3(i+i1), x1),
        dot(hashv3(i+i2), x2),
        dot(hashv3(i+1.0), x3)
    ));
    Result *= 21; //gives ~ -1..1
    Result = clamp(Result, -1, 1);
    Result = Result * 0.5 + 0.5; // 0..1
    return Result;
}


float valueNoise3D(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);

    f = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    float n000 = hash3(i + vec3(0,0,0));
    float n100 = hash3(i + vec3(1,0,0));
    float n010 = hash3(i + vec3(0,1,0));
    float n110 = hash3(i + vec3(1,1,0));
    float n001 = hash3(i + vec3(0,0,1));
    float n101 = hash3(i + vec3(1,0,1));
    float n011 = hash3(i + vec3(0,1,1));
    float n111 = hash3(i + vec3(1,1,1));

    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);

    float nxy0 = mix(nx00, nx10, f.y);
    float nxy1 = mix(nx01, nx11, f.y);

    // roughly uniform 0..1
    return mix(nxy0, nxy1, f.z);
}

float valueNoise2D(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    // Quintic interpolation
    f = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    float a = hashv2(i + vec2(0,0)).x;
    float b = hashv2(i + vec2(1,0)).x;
    float c = hashv2(i + vec2(0,1)).x;
    float d = hashv2(i + vec2(1,1)).x;

    float x1 = mix(a, b, f.x);
    float x2 = mix(c, d, f.x);

    // roughly equally 0..1
    return mix(x1, x2, f.y);
}

float GetGrassNoise(vec4 WorldPos, float GrassScale, float GrassQuantize){
    float GrassNoise = abs(valueNoise3D(WorldPos.xyz * GrassScale));
    GrassNoise += abs(valueNoise3D(-WorldPos.xyz * GrassScale * 2)) * 0.5;
    GrassNoise += 0.2;
    GrassNoise = int(GrassNoise * GrassQuantize) / GrassQuantize;
    GrassNoise = clamp(GrassNoise, 0, 1);
    return GrassNoise; 
}