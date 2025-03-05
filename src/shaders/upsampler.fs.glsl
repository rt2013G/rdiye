#version 420 core

uniform sampler2D source_texture;
uniform float filter_radius;

in vec2 tex_coords;

layout (location = 0) out vec3 upsample;

void main()
{
    float x = filter_radius;
    float y = filter_radius;

    vec3 a = texture(source_texture, vec2(tex_coords.x - x, tex_coords.y + y)).rgb;
    vec3 b = texture(source_texture, vec2(tex_coords.x,     tex_coords.y + y)).rgb;
    vec3 c = texture(source_texture, vec2(tex_coords.x + x, tex_coords.y + y)).rgb;
    vec3 d = texture(source_texture, vec2(tex_coords.x - x, tex_coords.y)).rgb;
    vec3 e = texture(source_texture, vec2(tex_coords.x,     tex_coords.y)).rgb;
    vec3 f = texture(source_texture, vec2(tex_coords.x + x, tex_coords.y)).rgb;
    vec3 g = texture(source_texture, vec2(tex_coords.x - x, tex_coords.y - y)).rgb;
    vec3 h = texture(source_texture, vec2(tex_coords.x,     tex_coords.y - y)).rgb;
    vec3 i = texture(source_texture, vec2(tex_coords.x + x, tex_coords.y - y)).rgb;

    upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;
}