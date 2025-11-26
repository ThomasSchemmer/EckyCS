#version 430 

out vec4 FragColor;

in vec4 WorldPos;
in vec4 WorldNormals;
in vec2 UV;

uniform vec3 BrushPos;
uniform float BrushSize;
uniform vec3 GlobalWorldPos;
uniform ivec2 TexSize;
uniform sampler2D ResultTex;

// 0 and 1 are vertex and normal buffer in vertex shader. 2 is count buffer, unused
layout(std430, binding = 3) buffer SelectionBuffer {
    uint Entries[];
} Selection;

const float Min = 0;
const float Max = 0.025;
const float BrushBorder = 0.25;

vec3 GetBrushColor(){
    float d = distance(WorldPos.xz, BrushPos.xz);
    float a0 = smoothstep(BrushSize - BrushBorder, BrushSize, d);
    float a1 = smoothstep(BrushSize, BrushSize + BrushBorder, d);
    vec3 BrushColor = vec3(a0 - a1);
    return BrushColor;

    ivec3 iWorld = ivec3(WorldPos) - ivec3(GlobalWorldPos);
    int Show = ((iWorld.x % 2) == 0) && ((iWorld.z % 2) == 0) ? 1 : 0;
    vec3 SelectionColor = texture(ResultTex, UV).xyz * Show;

    return clamp(SelectionColor + BrushColor, 0, 1);
}

float hash_uint_to_float(uint x) {
    x = (x ^ (x >> 16)) * 0x45d9f3b;
    x = (x ^ (x >> 16)) * 0x45d9f3b;
    x = (x ^ (x >> 16));
    return float(x) / float(0xFFFFFFFFU); // Normalize to [0, 1]
}

// Function to convert a uint to a vec3
vec3 hash(uint seed) {
    // Use three distinct hash values derived from the seed
    float x = hash_uint_to_float(seed);
    float y = hash_uint_to_float(seed + 12345U); // Offset to get different hash
    float z = hash_uint_to_float(seed + 67890U); // Offset to get different hash

    return vec3(x, y, z);
}

void main()
{
    float d = 1 - clamp(distance(BrushPos.xz, WorldPos.xz), 0, 1);
    vec3 Color = GetBrushColor();
    uint GlobalIndex = uint(UV.y * TexSize.y) * TexSize.x + uint(UV.x * TexSize.x);
    float Value = Selection.Entries[GlobalIndex];
    Value /= 256.0;
    FragColor = vec4(vec3(Value) + Color, 1);
} 