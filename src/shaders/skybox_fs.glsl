#version 330 core

out vec4 fragment_color;

in vec3 tex_coords;

uniform samplerCube skybox_cubemap;

void main() {
    fragment_color = texture(skybox_cubemap, tex_coords);
}