#ifndef RD_MODEL_H
#define RD_MODEL_H

#include "glad/glad.h"

// NOTE, TODO: using assimp at the moment to get something up and running quickly
//             in the future consider writing a custom loader
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <vector>
#include <unordered_map>
#include <iostream>

void ReplaceAll(std::string& input, std::string &old_char, std::string &new_char)
{
    size_t pos = 0; 
    while ((pos = input.find(old_char, pos)) != std::string::npos) 
    {
        input.replace(pos, old_char.size(), new_char); 
        pos += new_char.size();
    }
}
// NOTE: speeding up texture loading momentarily
// TODO: texture system/db
GLOBAL std::unordered_map<std::string, GLuint> global_loaded_textures;
GLuint LoadTexture(std::string filename)
{
    // NOTE: just a temporary fix to load some filenames
    // TODO: custom string library
    std::string old_char = "\\";
    std::string new_char = "/";
    ReplaceAll(filename, old_char, new_char);
    if (global_loaded_textures.find(filename) != global_loaded_textures.end()) {
        return(global_loaded_textures.at(filename));
    }
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    i32 texture_width, texture_height, channel_count;
    stbi_set_flip_vertically_on_load(true);

    std::string path = ASSETS_FOLDER + filename;
    u8 *data = stbi_load(path.c_str(), &texture_width, &texture_height, &channel_count, 0);
    if(data)
    {
        GLenum format;
        if(channel_count == 1)
        {
            format = GL_RED;
        }
        else if(channel_count == 3)
        {
            format = GL_RGB;
        }
        else if(channel_count == 4)
        {
            format = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 
                     0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        Assert(!"failed to load texture");
    }

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
    global_loaded_textures.insert({filename, texture_id});

    return(texture_id);
}

struct vertex_data
{
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
};

// NOTE, TODO: temporary just to make setting textures easier
struct pbr_texture_group
{
    GLuint albedo;
    GLuint normal;
    GLuint metallic;
    GLuint roughness;
    GLuint ao;
};

struct mesh_data
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    u32 index_count;
    u32 vertex_count;
    pbr_texture_group textures;
};

// NOTE: assuming albedo=0, normal=1, metallic=2, roughness=3, ao=4
void SetShaderPBRTextures(pbr_texture_group *pbr)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pbr->albedo);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pbr->normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pbr->metallic);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, pbr->roughness);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, pbr->ao);
}
void SetShaderPBRTextures(mesh_data *mesh)
{
    SetShaderPBRTextures(&mesh->textures);
}

mesh_data MeshData(u32 vertex_count, void* vertex_list, u32 index_count = 0, void* index_list = NULL)
{
    mesh_data m;
    m.index_count = index_count;
    m.vertex_count = vertex_count;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex_data), vertex_list, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void *)offsetof(vertex_data, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void *)offsetof(vertex_data, tex_coords));
    
    if(index_count > 0)
    {
        glGenBuffers(1, &m.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), index_list, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    return(m); 
}

mesh_data MeshDataUntextured(u32 vertex_count, void* vertex_list, u32 index_count = 0, void* index_list = NULL)
{
    mesh_data m;
    m.index_count = index_count;
    m.vertex_count = vertex_count;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vec3), vertex_list, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
    glBindVertexArray(0);

    if(index_count > 0)
    {
        glGenBuffers(1, &m.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), index_list, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    return(m); 
}

void RenderMesh(mesh_data *mesh)
{
    SetShaderPBRTextures(mesh);
    glBindVertexArray(mesh->vao);
    if(mesh->index_count > 0)
    {
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
    }
}

void ProcessNode(std::vector<mesh_data> &mesh_list, std::string directory, aiNode *node, const aiScene *scene)
{
    for(u32 mesh_index = 0; mesh_index < node->mNumMeshes; mesh_index++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[mesh_index]];

        u32 vertex_count = mesh->mNumVertices;
        // TODO, MEMORY
        vertex_data *vertex_list = (vertex_data*)malloc(sizeof(vertex_data) * vertex_count);
        u32 face_count = mesh->mNumFaces;
        u32 index_count = 0;
        for(u32 face_index = 0; face_index < face_count; face_index++)
        {
            index_count += mesh->mFaces[face_index].mNumIndices;
        } 
        u32 *index_list = (u32*)malloc(sizeof(u32) * index_count);

        for(u32 vertex_index = 0; vertex_index < vertex_count; vertex_index++)
        {
            vertex_data vertex;
            vertex.position = Vec3(mesh->mVertices[vertex_index].x, 
                                mesh->mVertices[vertex_index].y,
                                mesh->mVertices[vertex_index].z);
            vertex.normal = Vec3(mesh->mNormals[vertex_index].x, 
                                mesh->mNormals[vertex_index].y, 
                                mesh->mNormals[vertex_index].z);

            // NOTE, TODO: only allowing 1 texture coord per mesh at the moment
            if(mesh->mTextureCoords[0])
            {
                vertex.tex_coords = Vec2(mesh->mTextureCoords[0][vertex_index].x, 
                                        mesh->mTextureCoords[0][vertex_index].y);
            }
            else
            {
                vertex.tex_coords = Vec2(0.0f, 0.0f);
            }
            vertex_list[vertex_index] = vertex;
        }

        u32 total_index = 0;
        for(u32 face_index = 0; face_index < face_count; face_index++)
        {
            for(u32 index = 0; index < mesh->mFaces[face_index].mNumIndices; index++)
            {
                Assert(total_index < index_count);
                index_list[total_index] = mesh->mFaces[face_index].mIndices[index];
                total_index++;
            }
        }

        mesh_data m = MeshData(vertex_count, &vertex_list[0], index_count, &index_list[0]);
        // TODO: support meshes with multiple textures
        if(mesh->mMaterialIndex >= 0)
        {
            aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
            aiString filename;
            if(mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                mat->GetTexture(aiTextureType_DIFFUSE, 0, &filename);
            }
            else if(mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
            {
                mat->GetTexture(aiTextureType_BASE_COLOR, 0, &filename);
            }
            else
            {
                Assert("missing diffuse");
                // TODO: debug
            }
            std::string path = directory + '/' + filename.C_Str();
            m.textures.albedo = LoadTexture(path);

            if(mat->GetTextureCount(aiTextureType_NORMALS) > 0)
            {
                mat->GetTexture(aiTextureType_NORMALS, 0, &filename);
                path = directory + '/' + filename.C_Str();
                m.textures.normal = LoadTexture(path);
            }
            else if(mat->GetTextureCount(aiTextureType_HEIGHT) > 0)
            {
                mat->GetTexture(aiTextureType_HEIGHT, 0, &filename);
                path = directory + '/' + filename.C_Str();
                m.textures.normal = LoadTexture(path);
            }
            else
            {
                m.textures.normal = LoadTexture(TEXTURE_DEFAULT_NORMAL_MAP);
            }

            if(mat->GetTextureCount(aiTextureType_METALNESS) > 0)
            {
                mat->GetTexture(aiTextureType_METALNESS, 0, &filename);
                path = directory + '/' + filename.C_Str();
                m.textures.metallic = LoadTexture(path);
            }
            else
            {
                m.textures.metallic = LoadTexture(TEXTURE_DEFAULT_BLACK);
            }

            if(mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
            {
                mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &filename);
                path = directory + '/' + filename.C_Str();
                m.textures.roughness = LoadTexture(path);
            }
            else
            {
                m.textures.roughness = LoadTexture(TEXTURE_DEFAULT_BLACK);
            }
            
            if(mat->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
            {
                mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &filename);
                path = directory + '/' + filename.C_Str();
                m.textures.ao = LoadTexture(path);
            }
            else if(mat->GetTextureCount(aiTextureType_AMBIENT) > 0)
            {
                mat->GetTexture(aiTextureType_AMBIENT, 0, &filename);
                path = directory + '/' + filename.C_Str();
                m.textures.ao = LoadTexture(path);
            }
            else
            {
                m.textures.ao = LoadTexture(TEXTURE_DEFAULT_WHITE);
            }
        }
        mesh_list.push_back(m);
    }
    for(u32 child_index = 0; child_index < node->mNumChildren; child_index++)
    {
        ProcessNode(mesh_list, directory, node->mChildren[child_index], scene);
    }
}

void LoadModel(std::vector<mesh_data> &mesh_list, std::string path)
{
    std::string directory = path.substr(0, path.find_last_of('/'));
    path = ASSETS_FOLDER + path;
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | 
                                           aiProcess_FlipUVs | 
                                           aiProcess_GenSmoothNormals | 
                                           aiProcess_JoinIdenticalVertices);
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        // TODO: debug
    }
    ProcessNode(mesh_list, directory, scene->mRootNode, scene);
}

#endif