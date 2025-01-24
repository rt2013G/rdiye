#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

out vs_out
{
    vec3 fragment_position;
    vec3 normal;
    vec2 tex_coords;
} vertex_output;

uniform mat4 model;
uniform mat4 projection_mul_view;

void main()
{
    vec4 world_position = model * vec4(in_pos, 1.0f);
    vertex_output.fragment_position = world_position.xyz;
    vertex_output.normal = (model * vec4(in_normal, 1.0f)).xyz;
    vertex_output.tex_coords = in_tex_coords;
    
    gl_Position = projection_mul_view * world_position;
}