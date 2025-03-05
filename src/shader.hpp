#ifndef SHADER_HPP
#define SHADER_HPP

#include "glad/glad.h"

#include "fstream"
#include "iostream"
#include "sstream"
#include "string"

struct ShaderProgram {
    GLuint id;
    ShaderProgram(const char *vertex_shader_path, const char *fragment_shader_path, const char *geometry_shader_path);
    void use();
    void set_int(const char *name, i32 value);
    void set_float(const char *name, f32 value);
    void set_bool(const char *name, b32 value);
    void set_vec2(const char *name, vec2 vec);
    void set_vec3(const char *name, vec3 vec);
    void set_mat4(const char *name, mat4x4 mat);
    void set_mat3(const char *name, mat3x3 mat);
};

ShaderProgram::ShaderProgram(const char *vertex_shader_path, const char *fragment_shader_path, 
                             const char *geometry_shader_path = NULL) {
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

    GLuint geometry_shader;
    if(geometry_shader_path)
    {
        std::ifstream geometry_file{geometry_shader_path};
        std::stringstream geometry_stream;
        geometry_stream << geometry_file.rdbuf();
        std::string geometry_string = geometry_stream.str();
        const char *geometry_src = geometry_string.c_str();
        geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry_shader, 1, &geometry_src, NULL);
        glCompileShader(geometry_shader);
        glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            glGetShaderInfoLog(geometry_shader, LOG_LENGTH, NULL, info_log);
            std::cout << "GEOMETRY SHADER COMPILATION ERROR:\n"
                        << info_log << std::endl;
        } 
    }

    this->id = glCreateProgram();
    glAttachShader(this->id, vertex_shader);
    glAttachShader(this->id, fragment_shader);
    if(geometry_shader_path)
    {
        glAttachShader(this->id, geometry_shader);
    }
    glLinkProgram(this->id);
    glGetProgramiv(this->id, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(this->id, LOG_LENGTH, NULL, info_log);
        std::cout << "PROGRAM LINKING ERROR:\n"
                  << info_log << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteShader(geometry_shader);
}

void ShaderProgram::use()
{
    glUseProgram(id);
}

void ShaderProgram::set_int(const char *name, i32 value)
{
    glUniform1i(glGetUniformLocation(id, name), value);
}

void ShaderProgram::set_float(const char *name, f32 value) 
{
    glUniform1f(glGetUniformLocation(id, name), value);
}

void ShaderProgram::set_bool(const char* name, b32 value)
{
    glUniform1i(glGetUniformLocation(id, name), value);
}

void ShaderProgram::set_vec2(const char *name, vec2 vec)
{
   glUniform2fv(glGetUniformLocation(id, name), 1, &vec.e[0]); 
}

void ShaderProgram::set_vec3(const char *name, vec3 vec)
{
   glUniform3fv(glGetUniformLocation(id, name), 1, &vec.e[0]); 
}

void ShaderProgram::set_mat4(const char *name, mat4x4 mat)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_TRUE, &mat.e[0][0]);
}

void ShaderProgram::set_mat3(const char *name, mat3x3 mat)
{
    glUniformMatrix3fv(glGetUniformLocation(id, name), 1, GL_TRUE, &mat.e[0][0]);
}


#endif