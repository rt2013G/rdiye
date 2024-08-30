#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include "lib/stb/stb_image.h"

#include "iostream"

#include "camera.hpp"
#include "data.hpp"
#include "lighting.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

bool global_first_mouse = true;
float global_lastX = WINDOW_WIDTH / 2.0f;
float global_lastY = WINDOW_HEIGHT / 2.0f;
camera active_camera = camera(glm::vec3(0.0f, 0.0f, 3.0f));
float delta_time = 0.0f;
glm::mat4 projection = glm::perspective(glm::radians(active_camera.FOV), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_PLANE, FAR_PLANE);

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(active_camera.FOV), (float)width / (float)height, NEAR_PLANE, FAR_PLANE);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        active_camera.move(camera::FORWARD, delta_time);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        active_camera.move(camera::BACKWARD, delta_time);
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        active_camera.move(camera::LEFT, delta_time);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        active_camera.move(camera::RIGHT, delta_time);
    }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float posX = (float)xposIn;
    float posY = (float)yposIn;
    if (global_first_mouse) {
        global_lastX = posX;
        global_lastY = posY;
        global_first_mouse = false;
    }
    float offsetX = posX - global_lastX;
    float offsetY = global_lastY - posY;
    global_lastX = posX;
    global_lastY = posY;
    active_camera.process_mouse(offsetX, offsetY);
}

void scroll_callback(GLFWwindow *window, double offsetX, double offsetY) {
    active_camera.process_scroll((float)offsetY);
}

int main(void) {
    if (!glfwInit()) {
        std::cout << "failed to initialize glfw" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hewlloi worlsd", NULL, NULL);
    if (window == NULL) {
        std::cout << "failed to create window";
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "failed to initialize glad" << std::endl;
        exit(EXIT_FAILURE);
    }
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glEnable(GL_DEPTH_TEST);

    ShaderProgram lighting_shader("src/shaders/lighting_vs.glsl", "src/shaders/lighting_fs.glsl");
    ShaderProgram skybox_shader("src/shaders/skybox_vs.glsl", "src/shaders/skybox_fs.glsl");
    ShaderProgram material_shader("src/shaders/material_vs.glsl", "src/shaders/material_fs.glsl");

    Mesh skybox_mesh;
    load_skybox_mesh(skybox_mesh, sizeof(SKYBOX_VERTICES), &SKYBOX_VERTICES[0]);
    std::string cubemap_faces[] = {
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
    Mesh container_mesh;
    load_mesh(container_mesh, sizeof(CUBE_VERTICES), &CUBE_VERTICES[0]);

    Mesh wall_mesh;
    load_mesh(wall_mesh, sizeof(CUBE_VERTICES), &CUBE_VERTICES[0]);
    Material wall_mat;
    load_material(wall_mat, "brickwall.jpg", "brickwall.jpg", 128, "brickwall_n.jpg");

    Mesh plane_mesh;
    load_mesh(plane_mesh, sizeof(PLANE_VERTICES), PLANE_VERTICES);
    Material plane_mat;
    load_material(plane_mat, "wood.png");

    GLuint light_cube_VAO, light_cube_VBO;
    glGenVertexArrays(1, &light_cube_VAO);
    glGenBuffers(1, &light_cube_VBO);
    glBindVertexArray(light_cube_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, light_cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LIGHT_CUBE_VERTICES), LIGHT_CUBE_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    DirectionalLight dir_light;
    dir_light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);

    int32_t point_light_count = 3;
    PointLight point_lights[MAX_POINT_LIGHT_COUNT];
    glm::vec3 plight_positions[3] = {
        glm::vec3(1.2f, 1.0f, 2.0f),
        glm::vec3(1.2f, 3.0f, 1.0f),
        glm::vec3(1.2f, -3.0f, 1.0f),
    };
    load_point_lights(point_lights, plight_positions, point_light_count);

    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        process_input(window);

        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(active_camera.FOV), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = active_camera.view_matrix();
        glm::mat4 projection_mul_view = projection * view;

        lighting_shader.use();
        for (int32_t i = 0; i < point_light_count; i++) {
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
        material_shader.set_vec3("viewer_position", active_camera.position);
        set_shader_lighting_data(material_shader, dir_light, point_lights, point_light_count);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat3 normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        material_shader.set_mat3("normal_matrix", normal_matrix);
        material_shader.set_mat4("model", model);
        set_shader_material(material_shader, container_mat);
        draw_mesh(container_mesh, material_shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 2.0f, 2.0f));
        material_shader.set_mat4("model", model);
        normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        material_shader.set_mat3("normal_matrix", normal_matrix);
        set_shader_material(material_shader, wall_mat);
        draw_mesh(wall_mesh, material_shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        material_shader.set_mat4("model", model);
        normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        material_shader.set_mat3("normal_matrix", normal_matrix);
        set_shader_material(material_shader, plane_mat);
        draw_mesh(plane_mesh, material_shader);

        skybox_shader.use();
        view = glm::mat4(glm::mat3(active_camera.view_matrix()));
        projection_mul_view = projection * view;
        skybox_shader.set_mat4("projection_mul_view", projection_mul_view);
        draw_skybox(skybox_mesh, skybox_shader, skybox_texture);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}