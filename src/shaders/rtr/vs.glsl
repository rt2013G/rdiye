#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

out vec3 fragment_position;
out vec3 normal;
out vec2 tex_coords;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    normal = in_normal;
    tex_coords = in_tex_coords;
    fragment_position = vec3(transform * vec4(in_pos, 1.0f));
    gl_Position = projection * view * transform * vec4(in_pos, 1.0f);
}