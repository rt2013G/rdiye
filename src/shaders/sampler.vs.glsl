#version 420 core

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_tex_coords;

out vec2 tex_coords;

void main()
{
    tex_coords = in_tex_coords;
    gl_Position = vec4(in_pos.x, in_pos.y, 0.0f, 1.0f);
}