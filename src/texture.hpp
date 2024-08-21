#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "glad/glad.h"

#include "lib/stb/stb_image.h"

#include "iostream"
#include "string"

enum TextureType {
    DIFFUSE = 0,
    SPECULAR = 1,
    NORMAL = 2,
    PARALLAX = 3,
    ALBEDO = 4,
    METALLIC = 5,
    ROUGHNESS = 6,
    OCCLUSION = 7,
};
const std::string TEXTURE_TYPE_TO_STRING[]{
    "diffuse",
    "specular",
    "normal",
    "parallax",
    "albedo",
    "metallic",
    "roughness",
    "occlusion"};

struct Texture {
    GLuint id;
    TextureType type;
    std::string name;
    Texture(std::string path);
};

Texture::Texture(std::string path) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    int tex_width, tex_height, num_channels;
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
    } else {
        std::cout << "ERROR. Failed to load texture at path: " << path << std::endl;
        return;
    }
    stbi_image_free(data);
    std::size_t file_extension_pos = path.find_last_of(".");
    std::string texture_name_suffix = path.substr(file_extension_pos - 2, 2);
    TextureType tex_type = DIFFUSE;
    if (texture_name_suffix == "_s") {
        tex_type = SPECULAR;
    } else if (texture_name_suffix == "_n") {
        tex_type = NORMAL;
    } else if (texture_name_suffix == "_p") {
        tex_type = PARALLAX;
    } // TODO: do PBR textures
    std::size_t pos = path.find_last_of("/");
    std::string tex_name = path.substr(pos + 1, file_extension_pos - pos - 1);

    this->id = tex_id;
    this->type = tex_type;
    this->name = tex_name;
}

#endif