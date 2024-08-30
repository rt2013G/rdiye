#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "glad/glad.h"

#include "lib/stb/stb_image.h"

#include "iostream"
#include "string"
#include "unordered_map"

const static std::string ASSETS_FOLDER = "assets/";
static std::unordered_map<std::string, GLuint> global_loaded_textures;

GLuint load_texture(std::string filename) {
    if (global_loaded_textures.find(filename) != global_loaded_textures.end()) {
        return global_loaded_textures.at(filename);
    }
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    int tex_width, tex_height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    std::string path = ASSETS_FOLDER + filename;
    unsigned char *data = stbi_load(path.c_str(), &tex_width, &tex_height, &num_channels, 0);
    if (data) {
        GLenum format;
        if (num_channels == 1) {
            format = GL_RED;
        } else if (num_channels == 3) {
            format = GL_RGB;
        } else if (num_channels == 4) {
            format = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        std::cout << "ERROR. Failed to load texture at path: " << path << std::endl;
        stbi_set_flip_vertically_on_load(false);
        glBindTexture(GL_TEXTURE_2D, 0);
        return tex_id;
    }
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
    global_loaded_textures.insert({filename, tex_id});

    return tex_id;
}

GLuint load_cubemap(std::string *face_names, unsigned int face_len) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
    int tex_width, tex_height, num_channels;
    unsigned char *data;
    for (unsigned int i = 0; i < face_len; i++) {
        std::string path = ASSETS_FOLDER + face_names[i];
        data = stbi_load(path.c_str(), &tex_width, &tex_height, &num_channels, 0);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex_id;
}

struct Material {
    GLuint diffuse;
    GLuint specular;
    float shininess;
    GLuint normal_map;
    GLuint parallax_map;
};

void load_material(Material &material, std::string diffuse_name, std::string specular_name = "missing_texture.png", float shininess = 1, std::string normal_name = "default_normal_map.png", std::string parallax_name = "missing_texture.png") {
    material.diffuse = load_texture(diffuse_name);
    material.specular = load_texture(specular_name);
    material.shininess = shininess;
    material.normal_map = load_texture(normal_name);
    material.parallax_map = load_texture(parallax_name);
}

void set_shader_material(ShaderProgram &shader, Material &material) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse);
    shader.set_int("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular);
    shader.set_int("material.specular", 1);

    shader.set_float("material.shininess", material.shininess);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material.normal_map);
    shader.set_int("material.normal_map", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, material.parallax_map);
    shader.set_int("material.parallax_map", 3);

    glBindTexture(GL_TEXTURE_2D, 0);
}

struct PBRMaterial {
    // TODO: PBR
};

#endif