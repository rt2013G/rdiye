#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"

#include "defines.h"

struct CameraSettings {
    real32 alpha;
    real32 beta;
    real32 mov_speed;
    real32 sens_x, sens_y, sens_scroll;
    real32 FOV;
    real32 near_plane, far_plane;
};

static void load_default_camera_settings(CameraSettings *settings) {
    settings->alpha = -90.0f;
    settings->beta = 0.0f;
    settings->mov_speed = 2.5f;
    settings->sens_x = 0.1f;
    settings->sens_y = 0.1;
    settings->sens_scroll = 3.0f;
    settings->FOV = 45.0f;
    settings->near_plane = 0.05f;
    settings->far_plane = 100.0f;
}

struct Camera {
    enum direction {
        FORWARD = 0,
        BACKWARD = 1,
        LEFT = 2,
        RIGHT = 3,
    };
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    CameraSettings settings;
    Camera(glm::vec3 position, CameraSettings settings);
    Camera(glm::vec3 position);
    Camera(CameraSettings settings);
    Camera();
    glm::mat4 view_matrix();
    glm::mat4 perspective_projection(real32 window_width, real32 window_height);
    void update_vectors();
    void move(direction dir, real32 delta_time);
    void process_mouse(real32 offsetX, real32 offsetY);
    void process_scroll(real32 offsetY);
};

Camera::Camera(glm::vec3 position, CameraSettings settings) {
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    this->settings = settings;
    this->position = position;
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    update_vectors();
}

Camera::Camera(glm::vec3 position) {
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    load_default_camera_settings(&this->settings);
    this->position = position;
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    update_vectors();
}

Camera::Camera(CameraSettings settings) {
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    this->settings = settings;
    this->position = glm::vec3(0.0f, 0.0f, 0.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    update_vectors();
}

Camera::Camera() {
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    load_default_camera_settings(&this->settings);
    this->position = glm::vec3(0.0f, 0.0f, 0.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    update_vectors();
}

glm::mat4 Camera::view_matrix() {
    return glm::lookAt(this->position, this->position + this->front, this->up);
}

glm::mat4 Camera::perspective_projection(real32 window_width, real32 window_height) {
    glm::mat4 result = glm::perspective(glm::radians(this->settings.FOV), window_width / window_height, this->settings.near_plane, this->settings.far_plane);
    return result;
}

void Camera::update_vectors() {
    glm::vec3 tmp;
    tmp.x = cos(glm::radians(this->settings.alpha)) * cos(glm::radians(this->settings.beta));
    tmp.y = sin(glm::radians(this->settings.beta));
    tmp.z = sin(glm::radians(this->settings.alpha)) * cos(glm::radians(this->settings.beta));
    this->front = glm::normalize(tmp);
    this->right = glm::normalize(glm::cross(this->front, this->world_up));
    this->up = glm::normalize(glm::cross(this->right, this->front));
}

void Camera::move(direction dir, real32 delta_time) {
    real32 velocity = this->settings.mov_speed * delta_time;
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

void Camera::process_mouse(real32 offsetX, real32 offsetY) {
    offsetX *= this->settings.sens_x;
    offsetY *= this->settings.sens_y;
    this->settings.alpha += offsetX;
    this->settings.beta += offsetY;

    if (this->settings.beta > 89.0f) {
        this->settings.beta = 89.0f;
    } else if (this->settings.beta < -89.0f) {
        this->settings.beta = -89.0f;
    }
    update_vectors();
}

void Camera::process_scroll(real32 offsetY) {
    this->settings.FOV -= (real32)offsetY * this->settings.sens_scroll;
    if (this->settings.FOV < 1.0f) {
        this->settings.FOV = 1.0f;
    } else if (this->settings.FOV > 90.0f) {
        this->settings.FOV = 90.0f;
    }
}

#endif