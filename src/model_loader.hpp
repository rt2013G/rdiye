#ifndef MODEL_LOADER_HPP
#define MODEL_LOADER_HPP

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

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Mesh {
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    GLuint VAO, VBO, EBO;
    Mesh(std::vector<vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures);
    void draw(ShaderProgram &ShaderProgram);
};

Mesh::Mesh(std::vector<vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, tex_coords));
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

struct model_object {
    std::string path;
    std::string dir;
    std::vector<Mesh> meshes;
    std::vector<Texture> loaded_textures;
    model_object(std::string path);
    void process_node(aiNode *node, const aiScene *scene);
    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<texture> load_material_textures(aiMaterial *mat, aiTextureType type, texture_type tex_type);
    void draw(ShaderProgram &ShaderProgram);
};

model_object::model_object(std::string path) : path(path) {
    Assimp::Importer importer; // wtf is this java nonsense
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        std::cout << "ASSIMP LOAD ERROR:\n"
                  << importer.GetErrorString() << std::endl;
        return;
    }
    this->dir = path.substr(0, path.find_last_of('/'));

    process_node(scene->mRootNode, scene);
}

void model_object::process_node(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->process_mesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

std::vector<texture> model_object::load_material_textures(aiMaterial *mat, aiTextureType type, texture_type tex_type) {
    std::vector<texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < this->loaded_textures.size(); j++) {
            if (std::strcmp(this->loaded_textures[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(this->loaded_textures[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            texture tex;
            tex.id = load_texture(str.C_Str(), this->dir);
            tex.type = tex_type;
            tex.path = str.C_Str();
            textures.push_back(tex);
            this->loaded_textures.push_back(tex);
        }
    }
    return textures;
}

Mesh model_object::process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertex v;
        glm::vec3 tmp_vec3;
        tmp_vec3.x = mesh->mVertices[i].x;
        tmp_vec3.y = mesh->mVertices[i].y;
        tmp_vec3.z = mesh->mVertices[i].z;
        v.position = tmp_vec3;

        tmp_vec3.x = mesh->mNormals[i].x;
        tmp_vec3.y = mesh->mNormals[i].y;
        tmp_vec3.z = mesh->mNormals[i].z;
        v.normal = tmp_vec3;

        if (mesh->mTextureCoords[0]) {
            glm::vec2 tmp_vec2;
            tmp_vec2.x = mesh->mTextureCoords[0][i].x;
            tmp_vec2.y = mesh->mTextureCoords[0][i].y;
            v.tex_coords = tmp_vec2;
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
        std::vector<texture> diffuse_maps = load_material_textures(mat, aiTextureType_DIFFUSE, DIFFUSE);
        std::vector<texture> specular_maps = load_material_textures(mat, aiTextureType_SPECULAR, SPECULAR);
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
        textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
    }

    return Mesh(vertices, indices, textures);
}

void model_object::draw(ShaderProgram &ShaderProgram) {
    for (unsigned int i = 0; i < this->meshes.size(); i++) {
        meshes[i].draw(ShaderProgram);
    }
}

#endif