#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

out vec2 texture_coord;
out vec3 normal;
out vec3 fragment_position;

layout (std140, binding = 0) uniform matrices_ubo {
    mat4 projection_mul_view;
};
uniform mat3 normal_matrix;
uniform mat4 model;

void main() {
    fragment_position = vec3(model * vec4(in_pos, 1.0));
    texture_coord = in_texture;
    normal = normal_matrix * in_normal;
    gl_Position = projection_mul_view * vec4(fragment_position, 1.0);
}

