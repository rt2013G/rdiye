#version 420 core

#define CASCADE_COUNT 3

layout (triangles, invocations = 5) in;
layout (triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform light_space_matrices_ubo
{
    mat4 light_space_matrices[CASCADE_COUNT];
};

void main()
{
    for(int i = 0; i < 3; i++)
    {
        gl_Position = light_space_matrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}