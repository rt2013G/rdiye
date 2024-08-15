#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_texture;

out vec2 texture_coord;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(in_pos, 1.0);
    texture_coord = in_texture;
}

