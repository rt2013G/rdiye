#version 330 core

layout (location = 0) in vec3 in_pos;

out vec3 tex_coords;

uniform mat4 projection_mul_view;

void main() {
    tex_coords = in_pos;
    vec4 pos = projection_mul_view * vec4(in_pos, 1.0);
    gl_Position = pos.xyww;
}