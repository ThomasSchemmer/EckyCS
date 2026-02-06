#version 430


#include "TERRAIN_COMMON_SHADER"

layout(std430, binding = SSBO_LAYOUT_VERTICES) buffer VertexBuffer {
    vec4 Entries[];
} Vertices;


uniform mat4 Transform;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 LightView;
uniform mat4 LightProjection;

out vec4 WorldPos;
out vec2 UV;
out vec4 PosLightClip;

const int Size = 100;

void main() {
    // Todo: offset according to sine wave, maybe find a better/faster way 
    // to compute this

    vec4 pos = Vertices.Entries[gl_VertexID];

    WorldPos = Transform * vec4(pos.xyz, 1);
    UV = vec2(pos.xz / float(Size));
    gl_Position = Projection * View * WorldPos;
    // in Light space to get shadow info
    PosLightClip = LightProjection * LightView * WorldPos;
}