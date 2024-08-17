#version 330 core

out vec4 frag_color;

in vec2 texture_coord;
in vec3 normal;
in vec3 fragment_position;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 viewer_position;

void main() {
    vec3 ambient = vec3(texture(material.diffuse, texture_coord)) * light.ambient;

    vec3 normalized_normal = normalize(normal);
    vec3 light_direction = normalize(light.position - fragment_position);
    float diffuse_strength = max(dot(normalized_normal, light_direction), 0.0);
    vec3 diffuse = vec3(texture(material.diffuse, texture_coord)) * diffuse_strength * light.diffuse;

    vec3 view_direction = normalize(viewer_position - fragment_position);
    vec3 reflect_direction = reflect(-light_direction, normalized_normal);
    float specular_strength = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
    vec3 specular = vec3(texture(material.specular, texture_coord)) * specular_strength * light.specular;

    vec3 phong_lighting = (ambient + diffuse + specular);
    frag_color = vec4(phong_lighting, 1.0);
}