#version 420 core

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2DArray shadow_map;
uniform float near_plane;
uniform float far_plane;
uniform int layer;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    float result = (2.0f * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
    
    return(result);
}

void main()
{             
    float depth = texture(shadow_map, vec3(tex_coords, layer)).r;
    frag_color = vec4(vec3(depth), 1.0);
}