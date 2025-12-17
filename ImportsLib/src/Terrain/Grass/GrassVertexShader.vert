#version 430

// layout isnt only vertex, see @SpriteGeometryProvider!
layout(std430, binding = 0) buffer VertexBuffer {
    vec4 Entries[];
} Vertices;

layout(std430, binding = 1) buffer PositionBuffer {
    vec4 Entries[];
} Positions;

layout(std430, binding = 2) buffer NormalsBuffer {
    vec4 Entries[];
} Normals;

uniform mat4 Transform;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 Right;
uniform vec3 Up;
uniform mat4 LightView;
uniform mat4 LightProjection;

// used to lookup grass color
out vec4 BaseWorldPos;
out vec4 WorldNormals;
out vec2 UV;
out vec4 PosLightClip;

const vec3 WorldUpOffset = vec3(0, 0.5, 0);

void main()
{
    // every other Entry is an actual vertex position
    vec3 Vertex = Vertices.Entries[gl_VertexID * 2].xyz;
    vec3 Combined = Vertex.x * Right + Vertex.y * Up + WorldUpOffset;
    vec3 Offset = Positions.Entries[gl_InstanceID].xyz;
    vec4 pos = vec4(Combined + Offset, 1);
    UV = vec2(
        Vertices.Entries[gl_VertexID * 2 + 0].w, 
        1 - Vertices.Entries[gl_VertexID * 2 + 1].x
    );
    BaseWorldPos = Transform * vec4(Offset, 1);
    vec4 WorldPos = Transform * vec4(pos.xyz, 1);
    PosLightClip = LightProjection * LightView * BaseWorldPos;
    gl_Position = Projection * View * WorldPos;
    WorldNormals = vec4(1, 0, 0, 1);
}