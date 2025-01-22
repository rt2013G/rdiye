#version 420 core

in vec3 fragment_position;
in vec3 normal;

out vec4 frag_color;

uniform vec3 viewer_position;
uniform vec3 warm_color;
uniform vec3 ambient_color;

#define MAX_LIGHTS 16

struct point_light
{
    vec3 position;
    vec3 color;
};
uniform point_light lights[MAX_LIGHTS];
uniform int light_count;

void main()
{
    frag_color = vec4(ambient_color, 1.0f);

    vec3 n = normalize(normal);
    vec3 v = normalize(viewer_position - fragment_position);
    for(int i = 0; i < light_count; i++)
    {
        vec3 l = normalize(lights[i].position - fragment_position);
        vec3 r = reflect(-l, n);
        
        float s = clamp(100.0f * dot(r, v) - 97.0f, 0.0f, 1.0f);
        float ndl = clamp(dot(n, l), 0.0f, 1.0f);

        vec3 highlight = vec3(2.0f, 2.0f, 2.0f);
        frag_color.rgb += ndl * mix(warm_color, highlight, s);
    }
}