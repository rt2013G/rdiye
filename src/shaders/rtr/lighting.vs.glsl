#version 420 core

layout (location = 0) in vec3 in_pos;

uniform mat4 model;
uniform mat4 projection_mul_view;

void main() {
	gl_Position = projection_mul_view * model * vec4(in_pos, 1.0f);
}