#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

out vec3 fragment_position;
out vec3 normal;
out vec2 tex_coords;

uniform mat4 model;
uniform mat4 projection_mul_view;

void main()
{
    vec4 world_position = model * vec4(in_pos, 1.0f);
    fragment_position = world_position.xyz;
    normal = (model * vec4(in_normal, 1.0f)).xyz;
    tex_coords = in_tex_coords;
    gl_Position = projection_mul_view * world_position;
}