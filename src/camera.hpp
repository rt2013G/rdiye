#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "rdiye_lib.h"

struct game_camera_settings
{
    f32 alpha;
    f32 beta;
    f32 speed;
    vec2 mouse_sensitivity;
    f32 scroll_sensitivity;
    f32 FOV;
    f32 near_plane;
    f32 far_plane;
};

game_camera_settings DefaultCameraSettings()
{
    game_camera_settings result = {};
    result.alpha = -90.f;
    result.beta = 0.0f;
    result.speed = 2.5f;
    result.mouse_sensitivity = Vec2(0.1f, 0.1f);
    result.scroll_sensitivity = 3.0f;
    result.FOV = 45.0f;
    result.near_plane = 0.05f;
    result.far_plane = 100.0f; 

    return(result);
}

#if 0

struct game_camera
{
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    game_camera_settings settings;
};

void UpdateCameraVectors(game_camera *camera)
{
    vec3 new_front = Vec3(
        Cosine(DegreesToRadians(camera->settings.alpha)) * Cosine(DegreesToRadians(camera->settings.beta)),
        Sine(DegreesToRadians(camera->settings.beta)),
        Sine(DegreesToRadians(camera->settings.alpha)) * Cosine(DegreesToRadians(camera->settings.beta))
    );
    camera->front = Normalize(new_front);
    camera->right = Normalize(Cross(camera->front, camera->world_up));
    camera->up = Normalize(Cross(camera->right, camera->front));
}

game_camera DefaultCamera()
{
    game_camera result = {};
    result.position = Vec3(0.0f, 0.0f, 0.0f);
    result.front = Vec3(0.0f, 0.0f, -1.0f);
    result.up = Vec3(0.0f, 1.0f, 0.0f);
    result.world_up = result.up;
    result.right = Vec3(1.0f, 0.0f, 0.0f);
    result.settings = DefaultCameraSettings();

    return(result);
}
#else
struct game_camera {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    game_camera_settings settings;
    game_camera(glm::vec3 position, game_camera_settings settings);
    game_camera(glm::vec3 position);
    game_camera(game_camera_settings settings);
    game_camera();
};

game_camera::game_camera() {
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    this->settings = DefaultCameraSettings();
    this->position = glm::vec3(0.0f, 0.0f, 0.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->right = glm::vec3(1.0f, 0.0f, 0.0f);
}

void UpdateCameraVectors(game_camera *camera) {
    glm::vec3 tmp;
    tmp.x = cos(glm::radians(camera->settings.alpha)) * cos(glm::radians(camera->settings.beta));
    tmp.y = sin(glm::radians(camera->settings.beta));
    tmp.z = sin(glm::radians(camera->settings.alpha)) * cos(glm::radians(camera->settings.beta));
    camera->front = glm::normalize(tmp);
    camera->right = glm::normalize(glm::cross(camera->front, camera->world_up));
    camera->up = glm::normalize(glm::cross(camera->right, camera->front));
}
#endif

glm::mat4 CameraViewMatrix(game_camera *camera)
{
    glm::mat4 result = glm::lookAt(
        camera->position,
        camera->position + camera->front,
        camera->up
    );
    return(result);
}

glm::mat4 PerspectiveProjection(game_camera *camera, f32 window_width, f32 window_height) 
{
    glm::mat4 result = glm::perspective(
        DegreesToRadians(camera->settings.FOV),
        window_width / window_height,
        camera->settings.near_plane,
        camera->settings.far_plane
    );
    
    return(result);
}

void MoveCamera(game_camera *camera, CameraDirection dir, f32 delta_time) {
    f32 velocity = camera->settings.speed * delta_time;
    switch (dir) {
        case FORWARD: {
            camera->position += camera->front * velocity;
        } break;
        case BACKWARD: {
            camera->position -= camera->front * velocity;
        } break;
        case LEFT: {
            camera->position -= camera->right * velocity;
        } break;
        case RIGHT: {
            camera->position += camera->right * velocity;
        } break;
    }
}

void ProcessCameraMouse(game_camera *camera, vec2 offset) 
{
    offset = Hadamard(offset, camera->settings.mouse_sensitivity);
    camera->settings.alpha += offset.x;
    camera->settings.beta += offset.y;

    if (camera->settings.beta > 89.0f) {
        camera->settings.beta = 89.0f;
    } else if (camera->settings.beta < -89.0f) {
        camera->settings.beta = -89.0f;
    }

    UpdateCameraVectors(camera);
}

void ProcessCameraScroll(game_camera *camera, f32 offset) 
{
    camera->settings.FOV -= offset * camera->settings.scroll_sensitivity;
    if (camera->settings.FOV < 1.0f) {
        camera->settings.FOV = 1.0f;
    } else if (camera->settings.FOV > 90.0f) {
        camera->settings.FOV = 90.0f;
    }
}

#endif