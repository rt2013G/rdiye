#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb/stb_image.h"

#include "iostream"

#include "camera.hpp"
#include "data.hpp"
#include "shader.hpp"

#define WINDOW_WIDTH 768
#define WINDOW_HEIGHT 768
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

    shader_program object_shader("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
    shader_program lighting_shader("src/shaders/lighting_vs.glsl", "src/shaders/lighting_fs.glsl");

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint light_cube_VAO;
    glGenVertexArrays(1, &light_cube_VAO);
    glBindVertexArray(light_cube_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLuint texture1, texture2;
    glGenTextures(0, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int img_width, img_height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/container.jpg", &img_width, &img_height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "ERROR while loading texture";
    }
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load("assets/awesomeface.png", &img_width, &img_height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "ERROR while loading texture";
    }
    stbi_image_free(data);
    object_shader.use();
    object_shader.set_int("texture1", 0);
    object_shader.set_int("texture2", 1);

    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        process_input(window);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

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
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 projection_mul_view = projection * view;
        glm::mat3 normal_matrix = glm::mat3(model);
        normal_matrix = glm::inverse(normal_matrix);
        normal_matrix = glm::transpose(normal_matrix);
        object_shader.use();
        object_shader.set_mat3("normal_matrix", normal_matrix);
        object_shader.set_mat4("model", model);
        object_shader.set_mat4("projection_mul_view", projection_mul_view);
        object_shader.set_vec3("light_color", glm::vec3(1.0f, 1.0f, 1.0f));
        object_shader.set_vec3("light_position", light_pos);
        object_shader.set_vec3("viewer_position", active_camera.position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURED_CUBE_WITH_NORMALS), TEXTURED_CUBE_WITH_NORMALS, GL_STATIC_DRAW);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}