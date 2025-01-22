#version 420 core

in vec3 fragment_position;
in vec3 normal;

out vec4 frag_color;

uniform vec3 surface_color;
uniform vec3 view_vector;
uniform vec3 light_direction;

void main()
{
    vec3 cool = vec3(0, 0, 0.55) + 0.25f * surface_color;
    vec3 warm = vec3(0.3, 0.3, 0) + 0.25f * surface_color;
    vec3 highlight = vec3(1, 1, 1);
    float t = (dot(normal, light_direction) + 1.0f) / 2.0f;
    vec3 r = reflect(light_direction, normal);
    float s = clamp((100 * dot(r, view_vector) - 97), 0.0f, 1.0f);

    vec3 shaded = s * highlight + (1 - s) * (t * warm + (1 - t) * cool);
    frag_color = vec4(shaded, 1.0f);
}