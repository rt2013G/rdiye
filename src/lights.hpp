#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"

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

#endif