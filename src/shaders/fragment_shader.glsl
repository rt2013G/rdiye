#version 330 core

out vec4 frag_color;

in vec2 texture_coord;
in vec3 normal;
in vec3 fragment_position;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 viewer_position;

void main() {
    vec4 texture_color = mix(texture(texture1, texture_coord), texture(texture2, texture_coord), 0.2);

    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 normalized_normal = normalize(normal);
    vec3 light_direction = normalize(light_position - fragment_position);
    float diffuse_strength = max(dot(normalized_normal, light_direction), 0.0);
    vec3 diffuse = diffuse_strength * light_color;

    float specular_strength = 0.5;
    vec3 view_direction = normalize(viewer_position - fragment_position);
    vec3 reflect_direction = reflect(-light_direction, normalized_normal);
    float specular_value = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
    vec3 specular = specular_strength * specular_value * light_color;

    vec3 phong_lighting = (ambient + diffuse + specular) * texture_color.xyz;
    frag_color = vec4(phong_lighting, 1.0);
}