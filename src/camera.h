#ifndef CAMERA_H
#define CAMERA_H

// TODO: remove this later
enum CameraDirection 
{
    FORWARD = 0,
    BACKWARD = 1,
    LEFT = 2,
    RIGHT = 3,
    UP = 4,
    DOWN = 5,
};

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

struct game_camera
{
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    game_camera_settings settings;
};

game_camera DefaultCamera()
{
    game_camera result = {};
    result.settings = DefaultCameraSettings();
    result.position = Vec3(0.0f, 0.0f, 0.0f);
    result.front = Vec3(0.0f, 0.0f, -1.0f);
    result.up = Vec3(0.0f, 1.0f, 0.0f);
    result.world_up = result.up;
    result.right = Vec3(1.0f, 0.0f, 0.0f);

    return(result);
}

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

mat4x4 CameraViewMatrix(game_camera *camera)
{
    mat4x4 result = LookAt(camera->position, camera->position + camera->front, camera->up);

    return(result);
}

mat4x4 PerspectiveProjection(game_camera *camera, f32 window_width, f32 window_height)
{
    mat4x4 result = Perspective(camera->settings.FOV, window_width / window_height,
                                camera->settings.near_plane, camera->settings.far_plane);

    return(result);
}

void MoveCamera(game_camera *camera, CameraDirection dir, f32 delta_time) 
{
    f32 velocity = camera->settings.speed * delta_time;
    switch (dir) 
    {
        case FORWARD: 
        {
            camera->position += camera->front * velocity;
        } break;
        case BACKWARD: 
        {
            camera->position -= camera->front * velocity;
        } break;
        case LEFT: 
        {
            camera->position -= camera->right * velocity;
        } break;
        case RIGHT: 
        {
            camera->position += camera->right * velocity;
        } break;
        case UP: 
        {
            camera->position += camera->up * velocity;
        } break;
        case DOWN: 
        {
            camera->position -= camera->up * velocity;
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