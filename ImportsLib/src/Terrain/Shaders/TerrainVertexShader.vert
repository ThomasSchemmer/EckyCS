#version 430


layout(std430, binding = 0) buffer PositionBuffer {
    vec4 Entries[];
} Positions;

layout(std430, binding = 1) buffer NormalBuffer {
    vec4 Entries[];
} Normals;

uniform mat4 Transform; 
uniform mat4 View; 
uniform mat4 Projection; 
out vec4 WorldPos;
out vec4 WorldNormals;
out vec2 UV;

const int Size = 100;

void main()
{
    vec4 pos = Positions.Entries[gl_VertexID];
    vec4 normal = Normals.Entries[gl_VertexID / 3];
    WorldPos = Transform * vec4(pos.xyz, 1);
    WorldNormals = Transform * vec4(normal.xyz, 1);
    UV = vec2(pos.xz / float(Size));
    gl_Position = Projection * View * WorldPos;
}