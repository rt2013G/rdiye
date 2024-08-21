#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "../texture.hpp"

#include "cassert"
#include "iostream"

void test_texture() {
    Texture tex = Texture("assets/brickwall_n.jpg");
    assert(tex.id == 1);
    assert(tex.name == "brickwall_n");
    assert(tex.type == NORMAL);

    tex = Texture("assets/bricks2_p.jpg");
    assert(tex.id == 2);
    assert(tex.name == "bricks2_p");
    assert(tex.type == PARALLAX);
}

int main(void) {
    if (!glfwInit()) {
        std::cout << "failed to initialize glfw" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(512, 512, "testing...", NULL, NULL);
    if (window == NULL) {
        std::cout << "failed to create window";
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "failed to initialize glad" << std::endl;
        exit(EXIT_FAILURE);
    }

    test_texture();
    std::cout << "TESTS OK" << std::endl;
    return 0;
}