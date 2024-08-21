#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"

const float DEFAULT_ALPHA = -90.0f;
const float DEFAULT_BETA = 0.0f;
const float DEFAULT_SPEED = 2.5f;
const float DEFAULT_SENSITIVITY_X = 0.1f;
const float DEFAULT_SENSITIVITY_Y = 0.1f;
const float DEFAULT_SCROLL_SENSITIVITY = 3.0f;
const float DEFAULT_FOV = 45.0f;

struct camera {
    enum direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    float alpha;
    float beta;
    float movement_speed;
    float sensitivityX;
    float sensitivityY;
    float scroll_sensitivity;
    float FOV;
    camera(glm::vec3 position, glm::vec3 up, float alpha, float beta);
    glm::mat4 view_matrix();
    void update_vectors();
    void move(direction dir, float delta_time);
    void process_mouse(float offsetX, float offsetY);
    void process_scroll(float offsetY);
};

camera::camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float alpha = DEFAULT_ALPHA, float beta = DEFAULT_BETA) {
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    movement_speed = DEFAULT_SPEED;
    sensitivityX = DEFAULT_SENSITIVITY_X;
    sensitivityY = DEFAULT_SENSITIVITY_Y;
    scroll_sensitivity = DEFAULT_SCROLL_SENSITIVITY;
    FOV = DEFAULT_FOV;
    this->position = position;
    this->world_up = up;
    this->alpha = alpha;
    this->beta = beta;
    update_vectors();
}

glm::mat4 camera::view_matrix() {
    return glm::lookAt(this->position, this->position + this->front, this->up);
}

void camera::update_vectors() {
    glm::vec3 tmp;
    tmp.x = cos(glm::radians(this->alpha)) * cos(glm::radians(this->beta));
    tmp.y = sin(glm::radians(this->beta));
    tmp.z = sin(glm::radians(this->alpha)) * cos(glm::radians(this->beta));
    this->front = glm::normalize(tmp);
    this->right = glm::normalize(glm::cross(this->front, this->world_up));
    this->up = glm::normalize(glm::cross(this->right, this->front));
}

void camera::move(direction dir, float delta_time) {
    float velocity = this->movement_speed * delta_time;
    switch (dir) {
    case FORWARD: {
        this->position += this->front * velocity;
    } break;
    case BACKWARD: {
        this->position -= this->front * velocity;
    } break;
    case LEFT: {
        this->position -= this->right * velocity;
    } break;
    case RIGHT: {
        this->position += right * velocity;
    } break;
    }
}

void camera::process_mouse(float offsetX, float offsetY) {
    offsetX *= this->sensitivityX;
    offsetY *= this->sensitivityY;
    this->alpha += offsetX;
    this->beta += offsetY;

    if (this->beta > 89.0f) {
        this->beta = 89.0f;
    } else if (this->beta < -89.0f) {
        this->beta = -89.0f;
    }
    update_vectors();
}

void camera::process_scroll(float offsetY) {
    this->FOV -= (float)offsetY * this->scroll_sensitivity;
    if (this->FOV < 1.0f) {
        this->FOV = 1.0f;
    } else if (this->FOV > 90.0f) {
        this->FOV = 90.0f;
    }
}

#endif