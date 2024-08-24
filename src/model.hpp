#ifndef MODEL_HPP
#define MODEL_HPP

#include "glad/glad.h"

#include "shader.hpp"
#include "texture.hpp"

struct Mesh {
    GLuint VAO, VBO;
    uint32_t vertices_size;
};

void build_mesh(Mesh &mesh, uint32_t vertices_size, const float *vertices) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    mesh.vertices_size = vertices_size;
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void draw_mesh(Mesh &mesh, ShaderProgram &shader, Texture &diffuse, Texture &specular) {
    glActiveTexture(GL_TEXTURE0);
    diffuse.bind();
    shader.set_int("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    specular.bind();
    shader.set_int("material.specular", 1);
    glBindVertexArray(mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertices_size / (sizeof(float) * 8));
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

struct EBOMesh {
    GLuint VAO, VBO, EBO;
    uint32_t indices_len;
};

void build_EBOmesh(EBOMesh &mesh, float *vertices, uint32_t vertices_len, GLuint *indices, uint32_t indices_len) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_len, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices_len, indices, GL_STATIC_DRAW);
    mesh.indices_len = indices_len;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void draw_EBOmesh(EBOMesh &mesh, ShaderProgram &shader, Texture &diffuse, Texture &specular) {
    glActiveTexture(GL_TEXTURE0);
    diffuse.bind();
    shader.set_int("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    specular.bind();
    shader.set_int("material.specular", 1);
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices_len, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

#endif