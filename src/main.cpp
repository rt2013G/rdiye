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
#include "model.hpp"
#include "shader.hpp"
#include "texture.hpp"

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

    ShaderProgram object_shader("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
    ShaderProgram lighting_shader("src/shaders/lighting_vs.glsl", "src/shaders/lighting_fs.glsl");
    ShaderProgram skybox_shader("src/shaders/skybox_vs.glsl", "src/shaders/skybox_fs.glsl");
    ShaderProgram plane_shader("src/shaders/floor_vs.glsl", "src/shaders/floor_fs.glsl");
    ShaderProgram shadow_shader("src/shaders/shadow_map_vs.glsl", "src/shaders/shadow_map_fs.glsl");

    Texture container_diffuse = Texture("assets/container2.png");
    Texture container_specular = Texture("assets/container2_s.png");

    Material container_mat = {container_diffuse.id, container_specular.id, 128};

    Mesh container_mesh;
    load_mesh(container_mesh, sizeof(CUBE_VERTICES), &CUBE_VERTICES[0]);

    GLuint light_cube_VAO, light_cube_VBO;
    glGenVertexArrays(1, &light_cube_VAO);
    glGenBuffers(1, &light_cube_VBO);
    glBindVertexArray(light_cube_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, light_cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LIGHT_CUBE_VERTICES), LIGHT_CUBE_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    uint8_t plight_count = 3;
    PointLight *plights = (PointLight *)malloc(sizeof(PointLight) * plight_count);
    glm::vec3 *plight_positions = (glm::vec3 *)malloc(sizeof(glm::vec3) * plight_count);
    plight_positions[0] = glm::vec3(1.2f, 1.0f, 2.0f);
    plight_positions[1] = glm::vec3(1.2f, 3.0f, 1.0f);
    plight_positions[2] = glm::vec3(1.2f, -3.0f, 1.0f);
    create_white_point_lights(plights, plight_positions, plight_count);

    DirectionalLight dir_light;
    create_dir_light(dir_light);

    Mesh skybox_mesh;
    load_skybox_mesh(skybox_mesh, sizeof(SKYBOX_VERTICES), &SKYBOX_VERTICES[0]);
    std::string cubemap_faces[] = {
        "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg",
    };
    GLuint skybox_texture = load_cubemap_texture(cubemap_faces, 6);

    Mesh plane_mesh;
    load_mesh(plane_mesh, sizeof(PLANE_VERTICES), PLANE_VERTICES);
    Texture plane_diffuse = Texture("assets/wood.png");
    Material plane_mat = {plane_diffuse.id, plane_diffuse.id, 4};

    const uint16_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint shadow_map_FBO;
    glGenFramebuffers(1, &shadow_map_FBO);
    GLuint shadow_map;
    glGenTextures(1, &shadow_map);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float shadow_border_clamp[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, shadow_border_clamp);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Mesh wall_mesh;
    load_mesh(wall_mesh, sizeof(CUBE_VERTICES), &CUBE_VERTICES[0]);
    Texture wall_diffuse = Texture("assets/brickwall.jpg");
    Texture wall_normal = Texture("assets/brickwall_n.jpg");
    Material wall_mat = {wall_diffuse.id, container_specular.id, 128};

    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        process_input(window);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        const float light_near_plane = 1.0f, light_far_plane = 7.5f;
        glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, light_near_plane, light_far_plane);
        glm::mat4 light_view = glm::lookAt(glm::vec3(0.5f, 4.0f, 2.0f),
                                           glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 light_space_matrix = light_projection * light_view;
        shadow_shader.use();
        shadow_shader.set_mat4("light_space_matrix", light_space_matrix);
        shadow_shader.set_mat4("model", glm::mat4(1.0f));
        draw_material_mesh(container_mesh, shadow_shader, container_mat);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(active_camera.FOV), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = active_camera.view_matrix();

        lighting_shader.use();
        for (uint8_t i = 0; i < plight_count; i++) {
            glm::mat4 transform = glm::mat4(1.0f);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, plight_positions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            transform = projection * view * model * transform;
            lighting_shader.set_mat4("transform", transform);
            glBindVertexArray(light_cube_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection_mul_view = projection * view;
        glm::mat3 normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        object_shader.use();
        object_shader.set_mat3("normal_matrix", normal_matrix);
        object_shader.set_mat4("model", model);
        object_shader.set_mat4("projection_mul_view", projection_mul_view);
        object_shader.set_vec3("viewer_position", active_camera.position);
        set_shader_dir_light(object_shader, dir_light, glm::vec3(-0.2f, -1.0f, -0.3f));
        set_shader_point_lights(object_shader, plights, plight_positions, plight_count);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        object_shader.set_int("environment_cubemap", 2);
        draw_material_mesh(container_mesh, object_shader, container_mat);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 2.0f, 2.0f));
        object_shader.set_mat4("model", model);
        normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        object_shader.set_mat3("normal_matrix", normal_matrix);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, wall_normal.id);
        object_shader.set_int("normal_map", 3);
        draw_material_mesh(wall_mesh, object_shader, wall_mat);

        plane_shader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        plane_shader.set_mat4("model", model);
        plane_shader.set_mat3("normal_matrix", normal_matrix);
        plane_shader.set_mat4("projection_mul_view", projection_mul_view);
        plane_shader.set_vec3("viewer_position", active_camera.position);
        set_shader_dir_light(plane_shader, dir_light, glm::vec3(-0.2f, -1.0f, -0.3f));
        set_shader_point_lights(plane_shader, plights, plight_positions, plight_count);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, shadow_map);
        plane_shader.set_int("shadow_map", 2);
        plane_shader.set_mat4("light_space_matrix", light_space_matrix);
        draw_material_mesh(plane_mesh, plane_shader, plane_mat);

        skybox_shader.use();
        view = glm::mat4(glm::mat3(active_camera.view_matrix()));
        skybox_shader.set_mat4("projection", projection);
        skybox_shader.set_mat4("view", view);
        draw_skybox(skybox_mesh, skybox_shader, skybox_texture);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}