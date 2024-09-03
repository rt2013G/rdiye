#version 420 core

#define MAX_POINT_LIGHT_COUNT 16

out vec4 frag_color;

in VS_OUT {
    vec2 texture_coord;
    vec3 vertex_normal;
    vec3 fragment_position;
} SHADER_INPUT;

struct Material {
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
    sampler2D normal_map;
    sampler2D parallax_map;
};

struct DirectionalLight {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};

struct PointLight {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    float constant_factor;
    float linear_factor;
    float quadratic_factor;
};

uniform vec3 viewer_position;
uniform Material material;
uniform DirectionalLight dir_light;
uniform PointLight point_lights[MAX_POINT_LIGHT_COUNT];
uniform int point_light_count;
uniform samplerCube environment_cubemap;

vec3 get_directional_light_contribution(DirectionalLight dlight, vec3 normal, vec3 view_dir);
vec3 get_point_light_contribution(PointLight plight, vec3 normal, vec3 view_dir, vec3 fragment_pos);

void main() {
    vec3 sampled_normal = texture(material.normal_map, SHADER_INPUT.texture_coord).rgb;
    sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    vec3 view_direction = normalize(viewer_position - SHADER_INPUT.fragment_position);

    vec3 color_output = get_directional_light_contribution(dir_light, sampled_normal, view_direction);
    for(int i = 0; i < point_light_count; i++) {
        color_output += get_point_light_contribution(point_lights[i], sampled_normal, view_direction, SHADER_INPUT.fragment_position);
    }
    frag_color = vec4(color_output, 1.0);
}

vec3 get_directional_light_contribution(DirectionalLight dlight, vec3 normal, vec3 view_dir) {
    vec3 light_direction = normalize(-dlight.direction);
    float diffuse_strength = max(dot(normal, light_direction), 0.0);

    vec3 reflect_direction = reflect(-light_direction, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_direction), 0.0), material.shininess);

    vec3 ambient = dlight.ambient * vec3(texture(material.diffuse, SHADER_INPUT.texture_coord));
    vec3 diffuse = dlight.diffuse * diffuse_strength * vec3(texture(material.diffuse, SHADER_INPUT.texture_coord));
    vec3 specular = dlight.specular * specular_strength * vec3(texture(material.specular, SHADER_INPUT.texture_coord));
    vec3 result = ambient * material.ambient_color + diffuse * material.diffuse_color + specular * material.specular_color;
    return result;
}

vec3 get_point_light_contribution(PointLight plight, vec3 normal, vec3 view_dir, vec3 fragment_pos) {
    float dist = length(plight.position - SHADER_INPUT.fragment_position);
    float attenuation = 1.0 / (plight.constant_factor + plight.linear_factor * dist + plight.quadratic_factor * dist * dist);

    vec3 light_direction = normalize(plight.position - SHADER_INPUT.fragment_position);
    float diffuse_strength = max(dot(normal, light_direction), 0.0);

    vec3 halway_vec = normalize(light_direction + view_dir);
    float specular_strength = pow(max(dot(normal, halway_vec), 0.0), material.shininess);

    vec3 ambient = attenuation * plight.ambient * vec3(texture(material.diffuse, SHADER_INPUT.texture_coord));
    vec3 diffuse = attenuation * plight.diffuse * diffuse_strength * vec3(texture(material.diffuse, SHADER_INPUT.texture_coord));
    vec3 specular = attenuation * plight.specular * specular_strength * vec3(texture(material.specular, SHADER_INPUT.texture_coord));
    vec3 result = ambient * material.ambient_color + diffuse * material.diffuse_color + specular * material.specular_color;
    return result;
}