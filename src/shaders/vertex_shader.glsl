#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_texture;

out vec3 color_value;
out vec2 texture_coord;

void main() {
    gl_Position = vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
    color_value = in_color;
    texture_coord = in_texture;
}

