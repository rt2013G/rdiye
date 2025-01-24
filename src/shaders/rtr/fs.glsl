#version 420 core

in vec3 fragment_position;
in vec3 normal;
in vec2 tex_coords;

out vec4 frag_color;

uniform vec3 viewer_position;

#define MAX_LIGHTS 16

struct point_light
{
    vec3 position;
    vec3 color;
};
uniform point_light lights[MAX_LIGHTS];
uniform int light_count;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;

void main()
{
    vec3 ambient = vec3(0.05f, 0.05f, 0.05f);
    vec3 diffuse = texture(diffuse_texture, tex_coords).rgb;
    vec3 specular = texture(specular_texture, tex_coords).rgb;
    vec3 n = normalize(normal);
    vec3 view = normalize(fragment_position - viewer_position);
    
    vec3 final_color = ambient;
    for(int i = 0; i < light_count; i++)
    {
        vec3 light_sub_fragment = lights[i].position - fragment_position;
        vec3 light = normalize(light_sub_fragment);
        vec3 halway_vector = normalize(light + view);
        float diffuse_strength = clamp(dot(n, light), 0.0f, 1.0f);
        float specular_strength = pow(clamp(dot(n, halway_vector), 0.0f, 1.0f), 256);
        float dist = dot(light_sub_fragment, light_sub_fragment);
        float attenuation = 1.0f / (1.0f + (0.09f * dist) + (0.032f * dist));

        final_color += (diffuse * diffuse_strength * attenuation) + 
                      (specular * specular_strength * attenuation);
    }

    frag_color = vec4(final_color, 1.0f);
}