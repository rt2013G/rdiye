#ifndef MESH_HPP
#define MESH_HPP

#include "glad/glad.h"

#include "material.hpp"
#include "shader.hpp"

struct Mesh {
    GLuint VAO, VBO;
    uint32_t vertices_size;
};

void load_mesh(Mesh &mesh, uint32_t vertices_size, const float *vertices) {
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

void draw_mesh(Mesh &mesh, ShaderProgram &shader) {
    glBindVertexArray(mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertices_size / (sizeof(float) * 8));
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void draw_material_mesh(Mesh &mesh, ShaderProgram &shader, Material mat) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mat.diffuse);
    shader.set_int("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mat.specular);
    shader.set_int("material.specular", 1);
    shader.set_float("material.shininess", mat.shininess);
    glBindVertexArray(mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertices_size / (sizeof(float) * 8));
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

struct EBOMesh {
    GLuint VAO, VBO, EBO;
    uint32_t indices_len;
};

void load_EBOmesh(EBOMesh &mesh, uint32_t vertices_size, float *vertices, uint32_t indices_len, GLuint *indices) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len * sizeof(GLuint), indices, GL_STATIC_DRAW);
    mesh.indices_len = indices_len;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void draw_material_EBOmesh(EBOMesh &mesh, ShaderProgram &shader, Material mat) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mat.diffuse);
    shader.set_int("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mat.specular);
    shader.set_int("material.specular", 1);
    shader.set_float("material.shininess", mat.shininess);
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices_len, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void load_skybox_mesh(Mesh &mesh, uint32_t vertices_size, const float *vertices) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    mesh.vertices_size = vertices_size;
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

void draw_skybox(Mesh &skybox_mesh, ShaderProgram &shader, GLuint skybox_texture) {
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(skybox_mesh.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    shader.set_int("skybox_cubemap", 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

#endif