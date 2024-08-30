#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"

#include "shader.hpp"

#define MAX_POINT_LIGHT_COUNT 16

struct DirectionalLight {
    glm::vec3 ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 direction;
};

struct PointLight {
    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 position;
    float constant_factor = 1.0f;
    float linear_factor = 0.09f;
    float quadratic_factor = 0.032f;
};

void load_point_lights(PointLight *plights, glm::vec3 *positions, int32_t point_light_count) {
    assert(point_light_count < MAX_POINT_LIGHT_COUNT);
    for (int32_t i = 0; i < point_light_count; i++) {
        plights[i].position = positions[i];
    }
}

void set_shader_lighting_data(ShaderProgram &shader, DirectionalLight dir_light, PointLight *point_lights, int32_t point_light_count) {
    assert(point_light_count < MAX_POINT_LIGHT_COUNT);
    shader.set_vec3("dir_light.ambient", dir_light.ambient);
    shader.set_vec3("dir_light.diffuse", dir_light.diffuse);
    shader.set_vec3("dir_light.specular", dir_light.specular);
    shader.set_vec3("dir_light.direction", dir_light.direction);
    for (int32_t i = 0; i < point_light_count; i++) {
        std::string index = "point_lights[" + std::to_string(i) + "].";
        shader.set_vec3(index + "ambient", point_lights[i].ambient);
        shader.set_vec3(index + "diffuse", point_lights[i].diffuse);
        shader.set_vec3(index + "specular", point_lights[i].specular);
        shader.set_vec3(index + "position", point_lights[i].position);
        shader.set_float(index + "constant_factor", point_lights[i].constant_factor);
        shader.set_float(index + "linear_factor", point_lights[i].linear_factor);
        shader.set_float(index + "quadratic_factor", point_lights[i].quadratic_factor);
    }
    shader.set_int("point_light_count", point_light_count);
}

#endif