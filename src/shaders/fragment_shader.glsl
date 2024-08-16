#version 330 core
out vec4 frag_color;

in vec2 texture_coord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 object_color;
uniform vec3 light_color;

void main() {
    vec4 texture_color = mix(texture(texture1, texture_coord), texture(texture2, texture_coord), 0.2); 
    frag_color = texture_color * vec4(object_color * light_color, 1.0);
}