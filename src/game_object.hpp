#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"

#include "model.hpp"
#include "shader.hpp"

struct GameObject {
    glm::mat4 transform = glm::mat4(1.0f);
    glm::mat3 normal_matrix;
    ShaderProgram &shader;
    Model model;
    void update_normal_matrix();
    GameObject(ShaderProgram &shader, std::string model_path);
    GameObject(ShaderProgram &shader, std::string model_path, glm::mat4 transform);
    void render();
    void scale(glm::vec3 scaling);
    void move(glm::vec3 direction);
    void rotate(glm::vec3 rotation_axis, float angle);
};

GameObject::GameObject(ShaderProgram &shader, std::string model_path) : shader(shader) {
    this->update_normal_matrix();
    load_model(this->model, model_path);
}

GameObject::GameObject(ShaderProgram &shader, std::string model_path, glm::mat4 transform) : shader(shader) {
    this->transform = transform;
    this->update_normal_matrix();
    load_model(this->model, model_path);
}

void GameObject::update_normal_matrix() {
    this->normal_matrix = glm::mat3(this->transform);
    this->normal_matrix = glm::inverse(this->normal_matrix);
    this->normal_matrix = glm::transpose(this->normal_matrix);
    this->shader.set_mat3("normal_matrix", normal_matrix);
}

void GameObject::render() {
    this->shader.set_mat4("model", this->transform);
    this->shader.set_mat3("normal_matrix", this->normal_matrix);
    this->model.draw(this->shader);
}

void GameObject::scale(glm::vec3 scaling) {
    this->transform = glm::scale(this->transform, scaling);
    this->update_normal_matrix();
}

void GameObject::move(glm::vec3 direction) {
    this->transform = glm::translate(this->transform, direction);
    this->update_normal_matrix();
}

void GameObject::rotate(glm::vec3 rotation_axis, float angle) {
    this->transform = glm::rotate(this->transform, glm::radians(angle), glm::normalize(rotation_axis));
    this->update_normal_matrix();
}

#endif