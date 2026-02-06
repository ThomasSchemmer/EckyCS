#version 430

#include "TERRAIN_COMMON_SHADER"

// layout isnt only vertex, see @SpriteGeometryProvider!
layout(std430, binding = SSBO_LAYOUT_VERTICES) buffer VertexBuffer {
    vec4 Entries[];
} Vertices;

layout(std430, binding = SSBO_LAYOUT_POSITIONS) buffer PositionBuffer {
    vec4 Entries[];
} Positions;

layout(std430, binding = SSBO_LAYOUT_NORMALS) buffer NormalsBuffer {
    vec4 Entries[];
} Normals;

uniform mat4 Transform;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 Right;
uniform vec3 Up;
uniform mat4 LightView;
uniform mat4 LightProjection;
// calculating grass color, depending on underlying terrain
uniform float GrassScale;
uniform float GrassQuantize;
uniform sampler2D GrassTex;

// used to lookup grass color
out vec4 BaseWorldPos;
out vec4 WorldNormals;
out vec2 UV;
out vec4 PosLightClip;
out float GrassNoise;

const vec3 WorldUpOffset = vec3(0, 0.5, 0);

float GetGrassNoise(vec4 WorldPos, float GrassScale, float GrassQuantize);
#include "TERRAIN_NOISE_SHADER"

void main()
{
    // every other Entry is an actual vertex position
    vec3 Vertex = Vertices.Entries[gl_VertexID * 2].xyz;
    vec3 Combined = Vertex.x * Right + Vertex.y * Up + WorldUpOffset;
    vec4 Offset = Positions.Entries[gl_InstanceID];
    float DecorationType = Offset.w;
    vec4 pos = vec4(Combined + Offset.xyz, 1);
    UV = vec2(
        Vertices.Entries[gl_VertexID * 2 + 0].w, 
        1 - Vertices.Entries[gl_VertexID * 2 + 1].x
    );
    BaseWorldPos = Transform * vec4(Offset.xyz, 1);
    vec4 WorldPos = Transform * vec4(pos.xyz, 1);
    PosLightClip = LightProjection * LightView * BaseWorldPos;
    gl_Position = Projection * View * WorldPos;
    WorldNormals = vec4(1, 0, 0, 1);

    //todo: should only use a general noise input and then scale in f-shader prolly
    GrassNoise = GetGrassNoise(BaseWorldPos, GrassScale, GrassQuantize);
    // only set this now to still have light clip position correct!
    BaseWorldPos.w = DecorationType;
}