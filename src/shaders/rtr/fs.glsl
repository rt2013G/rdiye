#version 420 core

#define MAX_LIGHTS 16
struct point_light
{
    vec3 position;
};

in vs_out
{
    vec3 fragment_position;
    vec3 normal;
    vec2 tex_coords;
} vertex_output;

uniform vec3 viewer_position;
uniform vec3 directional_light;
uniform point_light lights[MAX_LIGHTS];
uniform int light_count;
uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform float shininess;

out vec4 frag_color;

vec3 DirectionalLightContribution(vec3 direction, vec3 normal, vec3 view_direction)
{
    vec3 light_direction = normalize(-direction);
    vec3 reflected_light = reflect(-light_direction, normal);

    float diffuse_strength = clamp(dot(normal, light_direction), 0.0f, 1.0f);
    float specular_strength = pow(max(dot(view_direction, reflected_light), 0.0f), shininess);

    vec3 sampled_diffuse = texture(diffuse_texture, vertex_output.tex_coords).rgb;
    vec3 sampled_specular = texture(specular_texture, vertex_output.tex_coords).rgb;

    vec3 ambient_light = vec3(0.05f, 0.05f, 0.05f);
    vec3 diffuse_light = vec3(0.4f, 0.4f, 0.4f);
    vec3 specular_light = vec3(0.5f, 0.5f, 0.5f);

    vec3 result = (ambient_light * sampled_diffuse) +
                  (diffuse_light * sampled_diffuse * diffuse_strength) +
                  (specular_light * sampled_specular * specular_strength);
    return(result);
}

vec3 PointLightContribution(vec3 light_position, vec3 normal, vec3 view_direction)
{
    float distance = length(light_position - vertex_output.fragment_position);
    float attenuation = 1.0f / (1.0f + (0.09f * distance) + 
                               (0.032f * distance * distance));

    vec3 light_direction = normalize(light_position - vertex_output.fragment_position);
    vec3 halway_vector = normalize(light_direction + view_direction);

    float diffuse_strength = clamp(dot(normal, light_direction), 0.0f, 1.0f);
    float specular_strength = pow(max(dot(normal, halway_vector), 0.0f), shininess);

    vec3 sampled_diffuse = texture(diffuse_texture, vertex_output.tex_coords).rgb;
    vec3 sampled_specular = texture(specular_texture, vertex_output.tex_coords).rgb;

    vec3 ambient_light = vec3(0.2f, 0.2f, 0.2f);
    vec3 diffuse_light = vec3(0.5f, 0.5f, 0.5f);
    vec3 specular_light = vec3(1.0f, 1.0f, 1.0f);

    vec3 result = (ambient_light * sampled_diffuse * attenuation) +
                  (diffuse_light * sampled_diffuse * diffuse_strength * attenuation) +
                  (specular_light * sampled_specular * specular_strength * attenuation);
    return(result);
}

void main()
{
    vec3 normal = normalize(vertex_output.normal);
    vec3 view_direction = normalize(viewer_position - vertex_output.fragment_position);

    vec3 final_color = DirectionalLightContribution(directional_light, normal, view_direction);
    for(int i = 0; i < light_count; i++)
    {
        final_color += PointLightContribution(lights[i].position, normal, view_direction);
    }

    frag_color = vec4(final_color, 1.0f);
}