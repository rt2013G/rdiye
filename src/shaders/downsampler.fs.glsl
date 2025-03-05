#version 420 core

uniform sampler2D source_texture;
uniform vec2 source_resolution;

in vec2 tex_coords;

layout (location = 0) out vec3 downsample;

void main()
{
    // TODO: we should probably compute 1/resolution once on the CPU
    //       and send that instead
    vec2 source_texel_size = 1.0f / source_resolution;
    float x = source_texel_size.x;
    float y = source_texel_size.y;

    vec3 a = texture(source_texture, vec2(tex_coords.x - 2*x, tex_coords.y + 2*y)).rgb;
    vec3 b = texture(source_texture, vec2(tex_coords.x,       tex_coords.y + 2*y)).rgb;
    vec3 c = texture(source_texture, vec2(tex_coords.x + 2*x, tex_coords.y + 2*y)).rgb;
    vec3 d = texture(source_texture, vec2(tex_coords.x - 2*x, tex_coords.y)).rgb;
    vec3 e = texture(source_texture, vec2(tex_coords.x,       tex_coords.y)).rgb;
    vec3 f = texture(source_texture, vec2(tex_coords.x + 2*x, tex_coords.y)).rgb;
    vec3 g = texture(source_texture, vec2(tex_coords.x - 2*x, tex_coords.y - 2*y)).rgb;
    vec3 h = texture(source_texture, vec2(tex_coords.x,       tex_coords.y - 2*y)).rgb;
    vec3 i = texture(source_texture, vec2(tex_coords.x + 2*x, tex_coords.y - 2*y)).rgb;
    vec3 j = texture(source_texture, vec2(tex_coords.x - x, tex_coords.y + y)).rgb;
    vec3 k = texture(source_texture, vec2(tex_coords.x + x, tex_coords.y + y)).rgb;
    vec3 l = texture(source_texture, vec2(tex_coords.x - x, tex_coords.y - y)).rgb;
    vec3 m = texture(source_texture, vec2(tex_coords.x + x, tex_coords.y - y)).rgb;

    downsample = e*0.125;
    downsample += (a+c+g+i)*0.03125;
    downsample += (b+d+f+h)*0.0625;
    downsample += (j+k+l+m)*0.125;
}