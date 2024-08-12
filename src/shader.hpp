#ifndef SHADER_HPP
#define SHADER_HPP

#include "glad/glad.h"

#include "fstream"
#include "iostream"
#include "sstream"
#include "string"

struct shader_program {
    GLuint program_id;
    shader_program(const char *vertex_shader_path, const char *fragment_shader_path);
};

shader_program::shader_program(const char *vertex_shader_path, const char *fragment_shader_path) {
    std::ifstream vertex_file{vertex_shader_path};
    std::stringstream vertex_stream;
    vertex_stream << vertex_file.rdbuf();
    std::string vertex_string = vertex_stream.str();
    const char *vertex_src = vertex_string.c_str();

    std::ifstream fragment_file{fragment_shader_path};
    std::stringstream fragment_stream;
    fragment_stream << fragment_file.rdbuf();
    std::string fragment_string = fragment_stream.str();
    const char *fragment_src = fragment_string.c_str();

    GLint ok;
    const uint16_t LOG_LENGTH = 1024;
    char info_log[LOG_LENGTH];
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(vertex_shader, LOG_LENGTH, NULL, info_log);
        std::cout << "VERTEX SHADER COMPILATION ERROR:\n"
                  << info_log << std::endl;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(fragment_shader, LOG_LENGTH, NULL, info_log);
        std::cout << "FRAGMENT SHADER COMPILATION ERROR:\n"
                  << info_log << std::endl;
    }

    this->program_id = glCreateProgram();
    glAttachShader(this->program_id, vertex_shader);
    glAttachShader(this->program_id, fragment_shader);
    glLinkProgram(this->program_id);
    glGetProgramiv(this->program_id, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(this->program_id, LOG_LENGTH, NULL, info_log);
        std::cout << "PROGRAM LINKING ERROR:\n"
                  << info_log << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

#endif