#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

out VS_OUT {
    vec2 texture_coord;
    vec3 vertex_normal;
    vec3 fragment_position;
} SHADER_OUTPUT;

uniform mat3 normal_matrix;
uniform mat4 model;
uniform mat4 projection_mul_view;

void main() {
    SHADER_OUTPUT.fragment_position = vec3(model * vec4(in_pos, 1.0));
    SHADER_OUTPUT.vertex_normal = normal_matrix * in_normal;
    SHADER_OUTPUT.texture_coord = in_texture;
    gl_Position = projection_mul_view * vec4(SHADER_OUTPUT.fragment_position, 1.0);
}

