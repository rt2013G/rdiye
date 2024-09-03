#version 420 core

out vec4 frag_color;

in vec2 texture_coord;
in vec3 normal;
in vec3 fragment_position;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant_factor;
    float linear_factor;
    float quadratic_factor;
};
#define POINT_LIGHT_COUNT 3

uniform Material material;
uniform DirectionalLight dir_light;
uniform PointLight point_lights[POINT_LIGHT_COUNT];
uniform vec3 viewer_position;
uniform samplerCube environment_cubemap;
uniform sampler2D normal_map;

vec3 calculate_directional_light(DirectionalLight dir_light, vec3 normal, vec3 view_direction);
vec3 calculate_point_light(PointLight p_light, vec3 normal, vec3 fragment_position, vec3 view_direction);

void main() {
    vec3 normalized_normal = texture(normal_map, texture_coord).rgb;
    normalized_normal = normalize(normalized_normal * 2.0 - 1.0);
    //vec3 normalized_normal = normalize(normal);
    vec3 view_direction = normalize(viewer_position - fragment_position);

    vec3 color_output = calculate_directional_light(dir_light, normalized_normal, view_direction);
    for (int i = 0; i < POINT_LIGHT_COUNT; i++) {
        color_output += calculate_point_light(point_lights[i], normalized_normal, fragment_position, view_direction);
    }
    float ratio_water = 1.00 / 1.33;
    float ratio_glass = 1.00 / 1.52;
    float ratio_diamond = 1.00 / 2.42;
    vec3 I = normalize(fragment_position - viewer_position);
    vec3 R = refract(I, normalized_normal, ratio_diamond);
    vec3 environ = texture(environment_cubemap, R).rgb;
    // vec3 environ = vec3(1.0);
    frag_color = vec4(color_output, 1.0) * vec4(environ, 1.0);
}

vec3 calculate_directional_light(DirectionalLight dir_light, vec3 normal, vec3 view_direction) {
    vec3 light_direction = normalize(-dir_light.direction);
    float diffuse_strength = max(dot(normal, light_direction), 0.0);

    vec3 reflect_direction = reflect(-light_direction, normal);
    float specular_strength = pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess);

    vec3 ambient = dir_light.ambient * vec3(texture(material.diffuse, texture_coord));
    vec3 diffuse = dir_light.diffuse * diffuse_strength * vec3(texture(material.diffuse, texture_coord));
    vec3 specular = dir_light.specular * specular_strength * vec3(texture(material.specular, texture_coord));
    return (ambient + (diffuse + specular));
}

vec3 calculate_point_light(PointLight p_light, vec3 normal, vec3 fragment_position, vec3 view_direction) {
    float distance = length(p_light.position - fragment_position);
    float attenuation = 1.0 / (p_light.constant_factor + p_light.linear_factor * distance + p_light.quadratic_factor * distance * distance);

    vec3 light_direction = normalize(p_light.position - fragment_position);
    float diffuse_strength = max(dot(normal, light_direction), 0.0);

    vec3 halfway_vector = normalize(light_direction + view_direction);
    float specular_strength = pow(max(dot(normal, halfway_vector), 0.0), material.shininess);

    vec3 ambient = vec3(texture(material.diffuse, texture_coord)) * p_light.ambient * attenuation;
    vec3 diffuse = vec3(texture(material.diffuse, texture_coord)) * diffuse_strength * p_light.diffuse * attenuation;
    vec3 specular = vec3(texture(material.specular, texture_coord)) * specular_strength * p_light.specular * attenuation;
    return (ambient + (diffuse + specular));
}
