#include "glad/glad.h"
// NOTE: include glad before glfw
#include "GLFW/glfw3.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include "lib/stb/stb_image.h"

#include "rdiye_lib.h"
#include "camera.h"

#include "data.hpp"
#include "game_object.hpp"
#include "lighting.hpp"
#include "material.hpp"
#include "model.hpp"
#include "shader.hpp"

struct game_state
{
    GLFWwindow *window;
    u16 window_width;
    u16 window_height;
    f64 last_time;
    f32 delta_time;
    b32 is_running;
    game_camera player_camera;
};

GLOBAL u16 window_width = 1920;
GLOBAL u16 window_height = 1080;
GLOBAL game_state state;
GLOBAL bool has_mouse_moved = false;
GLOBAL vec2 mouse_last_movement;

void ResizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    glViewport(0, 0, width, height);
    state.window_width = width;
    state.window_height = height;
}

void MouseCallback(GLFWwindow *window, f64 pos_x, f64 pos_y)
{
    vec2 p = Vec2((f32)pos_x, (f32)pos_y);
    if (!has_mouse_moved)
    {
        mouse_last_movement = p;
        has_mouse_moved = true;
    }
    vec2 offset = Vec2(p.x - mouse_last_movement.x, 
                       mouse_last_movement.y - p.y);
    mouse_last_movement = p;
    #if 1
        ProcessCameraMouse(&state.player_camera, offset);
    #else
        ProcessCameraMouse(&state.player_camera, offset.x, offset.y);
    #endif
}

void ProcessInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        state.is_running = false;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, FORWARD, state.delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, BACKWARD, state.delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, LEFT, state.delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, RIGHT, state.delta_time);
    }
}

void ScrollCallback(GLFWwindow *window, f64 offset_x, f64 offset_y)
{
    ProcessCameraScroll(&state.player_camera, (f32)offset_y);
}

void GameInit()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "hello!", NULL, NULL);
    if (window == NULL)
    {
        // TODO
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        // TODO
        return;
    }
    glViewport(0, 0, window_width, window_height);

    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, ResizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glEnable(GL_DEPTH_TEST);

    state.is_running = true;
    state.window_width = window_width;
    state.window_height = window_height;
    state.last_time = 0.0f;
    state.delta_time = 0.0f;
    state.window = window;
    state.player_camera = DefaultCamera();

    mouse_last_movement = Vec2(state.window_width / 2.0f, state.window_height / 2.0f);
}

#if 0
void GameRun(void)
{
    ShaderProgram lighting_shader("src/shaders/lighting_vs.glsl", "src/shaders/lighting_fs.glsl");
    ShaderProgram skybox_shader("src/shaders/skybox_vs.glsl", "src/shaders/skybox_fs.glsl");
    ShaderProgram material_shader("src/shaders/material_vs.glsl", "src/shaders/material_fs.glsl");
    ShaderProgram shadow_shader("src/shaders/shadow_map_vs.glsl", "src/shaders/shadow_map_fs.glsl");

    BasicMesh skybox_mesh;
    load_skybox_mesh(skybox_mesh, sizeof(SKYBOX_VERTICES), &SKYBOX_VERTICES[0]);
    std::string cubemap_faces[] =
    {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg",
    };
    GLuint skybox_texture = load_cubemap(cubemap_faces, 6);

    Material container_mat;
    load_material(container_mat, "container2.png", "container2_s.png", 128);
    BasicMesh container_mesh;
    load_basic_mesh(container_mesh, sizeof(CUBE_VERTICES), &CUBE_VERTICES[0]);

    BasicMesh wall_mesh;
    load_basic_mesh(wall_mesh, sizeof(CUBE_VERTICES), &CUBE_VERTICES[0]);
    Material wall_mat;
    load_material(wall_mat, "brickwall.jpg", "brickwall.jpg", 128, "brickwall_n.jpg");

    BasicMesh plane_mesh;
    load_basic_mesh(plane_mesh, sizeof(PLANE_VERTICES), PLANE_VERTICES);
    Material plane_mat;
    load_material(plane_mat, "wood.png");

    GLuint light_cube_VAO, light_cube_VBO;
    glGenVertexArrays(1, &light_cube_VAO);
    glGenBuffers(1, &light_cube_VBO);
    glBindVertexArray(light_cube_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, light_cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LIGHT_CUBE_VERTICES), LIGHT_CUBE_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);

    DirectionalLight dir_light;
    dir_light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);

    u8 point_light_count = 3;
    PointLight point_lights[MAX_POINT_LIGHT_COUNT];
    glm::vec3 plight_positions[3] =
    {
        glm::vec3(1.2f, 1.0f, 2.0f),
        glm::vec3(1.2f, 3.0f, 1.0f),
        glm::vec3(1.2f, -3.0f, 1.0f),
    };
    load_point_lights(point_lights, plight_positions, point_light_count);

    GameObject backpack = GameObject(material_shader, "backpack/backpack.obj");
    backpack.move(glm::vec3(2.0f, -2.0f, -4.0f));
    backpack.scale(glm::vec3(0.25f, 0.25f, 0.25f));
    while (state.is_running)
    {
        f32 current_time = glfwGetTime();
        // TODO: delta should be f64
        state.delta_time = current_time - state.last_time;
        state.last_time = current_time;

        ProcessInput(state.window);

        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = PerspectiveProjection(&state.player_camera, state.window_width, state.window_height);
        glm::mat4 view = CameraViewMatrix(&state.player_camera);
        glm::mat4 projection_mul_view = projection * view;
        #if 0
        glm::vec3 player_position = Vec3ToGlm(state.player_camera.position);
        #else
        glm::vec3 player_position = state.player_camera.position; 
        #endif

        lighting_shader.use();
        for (u8 i = 0; i < point_light_count; i++)
        {
            glm::mat4 transform = glm::mat4(1.0f);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, plight_positions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            transform = projection * view * model * transform;
            lighting_shader.set_mat4("transform", transform);
            glBindVertexArray(light_cube_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        material_shader.use();
        material_shader.set_mat4("projection_mul_view", projection_mul_view);
        material_shader.set_vec3("viewer_position", player_position);
        set_shader_lighting_data(material_shader, dir_light, point_lights, point_light_count);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat3 normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        material_shader.set_mat3("normal_matrix", normal_matrix);
        material_shader.set_mat4("model", model);
        draw_basic_mesh(container_mesh, material_shader, container_mat);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 2.0f, 2.0f));
        material_shader.set_mat4("model", model);
        normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        material_shader.set_mat3("normal_matrix", normal_matrix);
        draw_basic_mesh(wall_mesh, material_shader, wall_mat);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        material_shader.set_mat4("model", model);
        normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        material_shader.set_mat3("normal_matrix", normal_matrix);
        draw_basic_mesh(plane_mesh, material_shader, plane_mat);

        backpack.render();

        skybox_shader.use();
        view = glm::mat4(glm::mat3(CameraViewMatrix(&state.player_camera)));
        projection_mul_view = projection * view;
        skybox_shader.set_mat4("projection_mul_view", projection_mul_view);
        draw_skybox(skybox_mesh, skybox_shader, skybox_texture);

        glfwSwapBuffers(state.window);
        glfwPollEvents();
    }

    glfwTerminate();
}
#else
void GameRun()
{
    GLuint cube_vao, cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES_TEXTURED), &CUBE_VERTICES_TEXTURED[0], GL_STATIC_DRAW);
    GLsizei cube_triangles_count = sizeof(CUBE_VERTICES_TEXTURED) / sizeof(f32) * 8; 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)(3 * sizeof(f32)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)(6 * sizeof(f32)));
    glBindVertexArray(0);

    ShaderProgram gooch_shader = ShaderProgram("src/shaders/rtr/vs.glsl", "src/shaders/rtr/gooch_shading.fs.glsl");

    vec3 light_direction = Vec3(0.0f, 0.5f, -0.5f);
    vec3 surface_color = Vec3(1.0f, 0.1f, 0.1f);

    while(state.is_running)
    {
        f32 current_time = glfwGetTime();
        state.delta_time = current_time - state.last_time;
        state.last_time = current_time;

        ProcessInput(state.window);

        glClearColor(0.7, 0.7, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = PerspectiveProjection(&state.player_camera, state.window_width, state.window_height);
        glm::mat4 view = CameraViewMatrix(&state.player_camera);

        mat4x4 cube_position = Identity();
        mat4x4 T = Translation(1.0f, 1.0f, -4.0f);
        mat4x4 S = Scaling(0.5f, 0.5f, 0.5f);
        mat4x4 transform = T * S * cube_position;

        mat4x4 light_rotation = RotationX(DegreesToRadians(5)) * 
                                RotationY(DegreesToRadians(3)) * 
                                RotationZ(DegreesToRadians(2));

        light_direction = Normalize(light_rotation * light_direction);

        gooch_shader.use();
        gooch_shader.set_mat4("transform", Mat4ToGlm(transform));
        gooch_shader.set_mat4("view", view);
        gooch_shader.set_mat4("projection", projection);
        gooch_shader.set_vec3("surface_color", Vec3ToGlm(surface_color));
        gooch_shader.set_vec3("light_direction", Vec3ToGlm(light_direction));
        gooch_shader.set_vec3("view_vector", Vec3ToGlm(state.player_camera.front));

        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_triangles_count);
        glBindVertexArray(0);

        glfwSwapBuffers(state.window);
        glfwPollEvents(); 
    }
}
#endif

int main(void)
{
    GameInit();
    GameRun();
    return(0);
}