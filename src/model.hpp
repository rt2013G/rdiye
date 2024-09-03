#ifndef MODEL_HPP
#define MODEL_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "vector"

#include "material.hpp"
#include "shader.hpp"

struct BasicMesh {
    GLuint VAO, VBO;
    uint32_t vertices_size;
};

void load_basic_mesh(BasicMesh &mesh, uint32_t vertices_size, const float *vertices) {
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

void draw_basic_mesh(BasicMesh &mesh, ShaderProgram &shader, Material &mat) {
    set_material_in_shader(mat, shader);
    glBindVertexArray(mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertices_size / (sizeof(float) * 8));
    glBindVertexArray(0);
}

void load_skybox_mesh(BasicMesh &mesh, uint32_t vertices_size, const float *vertices) {
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

void draw_skybox(BasicMesh &skybox_mesh, ShaderProgram &skybox_shader, GLuint skybox_texture) {
    glDepthFunc(GL_LEQUAL);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    skybox_shader.set_int("skybox_cubemap", 0);
    glBindVertexArray(skybox_mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Mesh {
    GLuint VAO, VBO, EBO;
    uint32_t indices_count;
    Material material;
};

void load_mesh(Mesh &mesh, std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material) {
    mesh.material = material;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    mesh.indices_count = indices.size();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tex_coords));

    glBindVertexArray(0);
}

void draw_mesh(Mesh &mesh, ShaderProgram &shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.material.diffuse);
    shader.set_int("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh.material.specular);
    shader.set_int("material.specular", 1);

    shader.set_float("material.shininess", mesh.material.shininess);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mesh.material.normal_map);
    shader.set_int("material.normal_map", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mesh.material.parallax_map);
    shader.set_int("material.parallax_map", 3);

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

struct Model {
    std::vector<Mesh> meshes;
    std::string directory;
    void draw(ShaderProgram &shader);
};

void load_assimp_material(Material &material, aiMaterial *assimp_mat, std::string directory) {
    aiString texture_filename;
    std::string diffuse_name;
    std::string specular_name = "missing_texture.png";
    std::string normal_name = "default_normal_map.png";
    std::string parallax_name = "missing_texture.png";

    if (assimp_mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        assimp_mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_filename);
        diffuse_name = directory + '/' + texture_filename.C_Str();
    } else if (assimp_mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
        assimp_mat->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_filename);
        diffuse_name = directory + '/' + texture_filename.C_Str();
    } else {
        std::cout << "ERROR, texture missing diffuse: " << directory << std::endl;
    }
    if (assimp_mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        assimp_mat->GetTexture(aiTextureType_SPECULAR, 0, &texture_filename);
        specular_name = directory + '/' + texture_filename.C_Str();
    }
    if (assimp_mat->GetTextureCount(aiTextureType_NORMALS) > 0) {
        assimp_mat->GetTexture(aiTextureType_NORMALS, 0, &texture_filename);
        normal_name = directory + '/' + texture_filename.C_Str();
    }
    if (assimp_mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {
        assimp_mat->GetTexture(aiTextureType_HEIGHT, 0, &texture_filename);
        parallax_name = directory + '/' + texture_filename.C_Str();
    }

    float shininess = 1.0f;
    if (assimp_mat->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_FAILURE) {
        shininess = 1.0f;
    };

    load_material(material, diffuse_name, specular_name, shininess, normal_name, parallax_name);
}

Mesh process_model_mesh(Model &model, aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Material material;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.tex_coords = vec;
        } else {
            vertex.tex_coords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *assimp_material = scene->mMaterials[mesh->mMaterialIndex];
        load_assimp_material(material, assimp_material, model.directory);
    }

    Mesh m;
    load_mesh(m, vertices, indices, material);
    return m;
}

void process_model_node(Model &model, aiNode *node, const aiScene *scene) {
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.push_back(process_model_mesh(model, mesh, scene));
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        process_model_node(model, node->mChildren[i], scene);
    }
}

void load_model(Model &model, std::string path) {
    model.directory = path.substr(0, path.find_last_of('/'));
    path = ASSETS_FOLDER + path;
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR with assimp:: " << importer.GetErrorString() << std::endl;
        return;
    }
    process_model_node(model, scene->mRootNode, scene);
}

void Model::draw(ShaderProgram &shader) {
    for (Mesh &m : this->meshes) {
        draw_mesh(m, shader);
    }
}

#endif