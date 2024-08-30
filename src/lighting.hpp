#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"

#include "material.hpp"
#include "shader.hpp"

struct DirectionalLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

void create_dir_light(DirectionalLight &dir_light) {
    dir_light.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    dir_light.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    dir_light.specular = glm::vec3(0.5f, 0.5f, 0.5f);
}

void set_shader_dir_light(ShaderProgram &shader, DirectionalLight &dir_light, glm::vec3 direction) {
    shader.set_vec3("dir_light.direction", direction);
    shader.set_vec3("dir_light.ambient", dir_light.ambient);
    shader.set_vec3("dir_light.diffuse", dir_light.diffuse);
    shader.set_vec3("dir_light.specular", dir_light.specular);
}

struct PointLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant_factor;
    float linear_factor;
    float quadratic_factor;
};

void create_white_point_lights(PointLight *plights, glm::vec3 *positions, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        plights[i].ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        plights[i].diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
        plights[i].specular = glm::vec3(1.0f, 1.0f, 1.0f);
        plights[i].constant_factor = 1.0f;
        plights[i].linear_factor = 0.09f;
        plights[i].quadratic_factor = 0.032f;
    }
}

void set_shader_point_lights(ShaderProgram &shader, PointLight *plights, glm::vec3 *positions, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        std::string index = "point_lights[" + std::to_string(i) + "].";
        shader.set_vec3(index + "position", positions[i]);
        shader.set_vec3(index + "ambient", plights[i].ambient);
        shader.set_vec3(index + "diffuse", plights[i].diffuse);
        shader.set_vec3(index + "specular", plights[i].specular);
        shader.set_float(index + "constant_factor", plights[i].constant_factor);
        shader.set_float(index + "linear_factor", plights[i].linear_factor);
        shader.set_float(index + "quadratic_factor", plights[i].quadratic_factor);
    }
}

#endif