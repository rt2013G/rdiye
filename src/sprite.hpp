#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"

#include "data.hpp"
#include "shader.hpp"
#include "texture.hpp"

struct Sprite {
    glm::vec2 size;
    glm::vec2 position;
    glm::mat4 transform;
    glm::vec3 color;
    GLuint VBO, VAO;
    Sprite(glm::vec2 position, glm::vec2 size, glm::vec3 color);
    void translate(glm::vec2 direction);
    void rotate(float angle);
    void scale(glm::vec2 size);
    void draw(Texture &tex, ShaderProgram &shader);
};

Sprite::Sprite(glm::vec2 size, glm::vec2 position, glm::vec3 color) {
    this->size = size;
    this->position = position;
    this->color = color;
    this->transform = glm::translate(glm::mat4(1.0f), glm::vec3(position, 1.0f));
    this->transform = glm::scale(this->transform, glm::vec3(size, 1.0f));
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SPRITE_VERTICES), SPRITE_VERTICES, GL_STATIC_DRAW);
    glBindVertexArray(this->VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Sprite::translate(glm::vec2 direction) {
    this->transform = glm::translate(this->transform, glm::vec3(direction, 1.0f));
}

void Sprite::rotate(float angle) {
    this->transform = glm::rotate(this->transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Sprite::scale(glm::vec2 size) {
    this->transform = glm::scale(this->transform, glm::vec3(size, 1.0f));
}

void Sprite::draw(Texture &tex, ShaderProgram &shader) {
    shader.use();
    shader.set_mat4("transform", this->transform);
    shader.set_vec3("sprite_color", this->color);
    glActiveTexture(GL_TEXTURE0);
    tex.bind();
    shader.set_int("sprite_texture", 0);
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

#endif