#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include "lib/stb/stb_image.h"

#include "iostream"

#include "camera.hpp"
#include "data.hpp"
#include "model_loader.hpp"
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
    ShaderProgram shadow_shader("src/shaders/empty_vs.glsl", "src/shaders/shadow_fs.glsl");

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint light_cube_VAO;
    glGenVertexArrays(1, &light_cube_VAO);
    glBindVertexArray(light_cube_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLuint plane_VBO, plane_VAO;
    glGenVertexArrays(1, &plane_VAO);
    glGenBuffers(1, &plane_VBO);
    glBindVertexArray(plane_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, plane_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURED_PLANE), TEXTURED_PLANE, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindVertexArray(0);

    GLuint material_diffuse_map, material_specular_map;
    glGenTextures(0, &material_diffuse_map);
    glBindTexture(GL_TEXTURE_2D, material_diffuse_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int img_width, img_height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/container2.png", &img_width, &img_height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "ERROR while loading texture";
    }
    glGenTextures(1, &material_specular_map);
    glBindTexture(GL_TEXTURE_2D, material_specular_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load("assets/container2_s.png", &img_width, &img_height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "ERROR while loading texture";
    }
    GLuint wood_texture = load_texture("assets/wood.png");
    stbi_image_free(data);

    float last_time = 0.0f;

    // model_object backpack = model_object("assets/backpack/backpack.obj");

    GLuint depth_map_FBO;
    glGenFramebuffers(1, &depth_map_FBO);

    GLuint depth_map;
    glGenTextures(1, &depth_map);
    glBindTexture(GL_TEXTURE_2D, depth_map);
    const uint16_t SHADOW_WIDTH = 1024;
    const uint16_t SHADOW_HEIGHT = 1024;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        process_input(window);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        const float LIGHT_NEAR_PLANE = 1.0f;
        const float LIGHT_FAR_PLANE = 7.5f;
        glm::mat4 light_render_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, LIGHT_NEAR_PLANE, LIGHT_FAR_PLANE);
        glm::mat4 light_render_view = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
                                                  glm::vec3(0.0f, 0.0f, 0.0f),
                                                  glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 light_space_matrix = light_render_projection * light_render_view;
        shadow_shader.use();
        shadow_shader.set_mat4("light_space_matrix", light_space_matrix);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material_diffuse_map);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material_specular_map);

        projection = glm::perspective(glm::radians(active_camera.FOV), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_PLANE, FAR_PLANE);
        glm::mat4 view = active_camera.view_matrix();

        glm::vec3 light_pos = glm::vec3(1.2f, 1.0f, 2.0f);
        glm::mat4 transform = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.2f));
        lighting_shader.use();
        transform = projection * view * model * transform;
        lighting_shader.set_mat4("transform", transform);
        glBufferData(GL_ARRAY_BUFFER, sizeof(PLAIN_CUBE_VERTICES), PLAIN_CUBE_VERTICES, GL_STATIC_DRAW);
        glBindVertexArray(light_cube_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        transform = glm::mat4(1.0f);
        model = glm::mat4(1.0f);
        glm::mat4 projection_mul_view = projection * view;
        glm::mat3 normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        object_shader.use();
        object_shader.set_mat3("normal_matrix", normal_matrix);
        object_shader.set_mat4("model", model);
        object_shader.set_mat4("projection_mul_view", projection_mul_view);
        object_shader.set_vec3("viewer_position", active_camera.position);
        object_shader.set_int("material.diffuse", 0);
        object_shader.set_int("material.specular", 1);
        object_shader.set_float("material.shininess", 128.0f);
        object_shader.set_vec3("dir_light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        object_shader.set_vec3("dir_light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        object_shader.set_vec3("dir_light.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        object_shader.set_vec3("dir_light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        object_shader.set_vec3("point_lights[0].position", light_pos);
        object_shader.set_vec3("point_lights[0].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
        object_shader.set_vec3("point_lights[0].diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        object_shader.set_vec3("point_lights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        object_shader.set_float("point_lights[0].constant", 1.0f);
        object_shader.set_float("point_lights[0].linear", 0.09f);
        object_shader.set_float("point_lights[0].quadratic", 0.032f);
        object_shader.set_mat4("light_space_matrix", light_space_matrix);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURED_CUBE_WITH_NORMALS), TEXTURED_CUBE_WITH_NORMALS, GL_STATIC_DRAW);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // backpack.draw(object_shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}