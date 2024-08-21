#ifndef MODEL_HPP
#define MODEL_HPP

#include "glad/glad.h"

#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include "lib/stb/stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "vector"

#include "shader.hpp"
#include "texture.hpp"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    GLuint VAO, VBO, EBO;
    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures);
    void draw(ShaderProgram &ShaderProgram);
};

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tex_coords));
    glBindVertexArray(0);
}

void Mesh::draw(ShaderProgram &ShaderProgram) {
    GLuint diffuse_count = 1;
    GLuint specular_count = 1;
    for (unsigned int i = 0; i < this->textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        TextureType type = this->textures[i].type;
        std::string texture_number;
        if (type == DIFFUSE) {
            texture_number = std::to_string(diffuse_count);
            diffuse_count++;
        } else if (type == SPECULAR) {
            texture_number = std::to_string(specular_count);
            specular_count++;
        }
        ShaderProgram.set_int(("material." + TEXTURE_TYPE_TO_STRING[type] + texture_number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

struct Model {
    const std::string path;
    std::string dir;
    std::vector<Mesh> meshes;
    std::vector<Texture> loaded_textures;
    std::vector<std::string> loaded_textures_paths;
    Model(std::string path);
    void build_mesh_tree(aiNode *node, const aiScene *scene);
    std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, TextureType tex_type);
    void draw(ShaderProgram &shader_program);
};

Model::Model(std::string path) : path(path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        std::cout << "ASSIMP LOAD ERROR:\n"
                  << importer.GetErrorString() << std::endl;
        return;
    }
    this->dir = path.substr(0, path.find_last_of('/'));
    build_mesh_tree(scene->mRootNode, scene);
}

void Model::build_mesh_tree(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            if (mesh->mTextureCoords[0]) {
                v.tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
                v.tex_coords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(v);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuse_maps = this->load_material_textures(mat, aiTextureType_DIFFUSE, DIFFUSE);
            std::vector<Texture> specular_maps = this->load_material_textures(mat, aiTextureType_SPECULAR, SPECULAR);
            textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
            textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
        }
        this->meshes.push_back(Mesh(vertices, indices, textures));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        build_mesh_tree(node->mChildren[i], scene);
    }
}

std::vector<Texture> Model::load_material_textures(aiMaterial *mat, aiTextureType type, TextureType tex_type) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool already_loaded = false;
        for (unsigned int j = 0; j < this->loaded_textures.size(); j++) {
            if (std::strcmp(this->loaded_textures_paths[j].data(), str.C_Str()) == 0) {
                textures.push_back(this->loaded_textures[j]);
                already_loaded = true;
                break;
            }
        }
        if (!already_loaded) {
            std::string path = this->dir + '/' + str.C_Str();
            Texture tex = Texture(path);
            tex.type = tex_type;
            textures.push_back(tex);
            this->loaded_textures.push_back(tex);
            this->loaded_textures_paths.push_back(str.C_Str());
        }
    }
    return textures;
}

void Model::draw(ShaderProgram &shader_program) {
    for (unsigned int i = 0; i < this->meshes.size(); i++) {
        this->meshes[i].draw(shader_program);
    }
}

#endif