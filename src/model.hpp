#ifndef MODEL_HPP
#define MODEL_HPP

#include "glad/glad.h"

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
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
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
    set_material_in_shader(mesh.material, shader);
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
    std::string specular_name = TEXTURE_DEFAULT_BLACK;
    std::string normal_name = TEXTURE_DEFAULT_NORMAL_MAP;
    std::string parallax_name = TEXTURE_DEFAULT_BLACK;

    if (assimp_mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        assimp_mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_filename);
        diffuse_name = directory + '/' + texture_filename.C_Str();
    } else if (assimp_mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
        assimp_mat->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_filename);
        diffuse_name = directory + '/' + texture_filename.C_Str();
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

    aiColor3D color(0.0f, 0.0f, 0.0f);

    assimp_mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
    vec3 ambient_color = Vec3(color.r, color.g, color.b);

    assimp_mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    vec3 diffuse_color = Vec3(color.r, color.g, color.b);

    assimp_mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
    vec3 specular_color = Vec3(color.r, color.g, color.b);

    float shininess = 1.0f;
    assimp_mat->Get(AI_MATKEY_SHININESS, shininess);

    load_material(material, diffuse_name, specular_name, shininess, normal_name, parallax_name, ambient_color, diffuse_color, specular_color);
}

Mesh process_model_mesh(Model &model, aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Material material;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0]) {
            vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.tex_coords = vec;
        } else {
            vertex.tex_coords = Vec2(0.0f, 0.0f);
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
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
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