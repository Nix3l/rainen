#include "texture.h"
#include "util/log.h"
#include "platform/platform.h"

static void load_image_stb(char* filepath, GLenum target, texture_s* texture) {
    stbi_set_flip_vertically_on_load(true);
    i32 width, height, num_channels;
    // TODO(nix3l): make this use stbi_load_from_memory so we actually make use of the permenant data 
    unsigned char* data = stbi_load(filepath, &width, &height, &num_channels, 0);

    if(!data) {
        LOG_ERR("couldnt load texture data from texture [%s]\n", filepath);
        LOG_ERR("failure reason: %s\n", stbi_failure_reason());
        return;
    }

    GLenum internal_format, format;

    // always assume 16 bit float values because who even cares
    if(num_channels == 1) {
        format = GL_RED;
        internal_format = GL_R16F;
    } else if(num_channels == 2) {
        format = GL_RG;
        internal_format = GL_RG16F;
    } else if(num_channels == 3) {
        format = GL_RGB;
        internal_format = GL_RGB16F;
    } else if(num_channels == 4) {
        format = GL_RGBA;
        internal_format = GL_RGBA16F;
    } else {
        LOG_ERR("error loading num of channels in texture [%s]\n", filepath);
        stbi_image_free(data);
        return;
    }

    glTexImage2D(
            target,
            0,
            internal_format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data);

    texture->width = width;
    texture->height = width;
    texture->num_channels = num_channels;
    texture->data_type = format;

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
}

static GLint get_internal_format(texture_s* texture) {
    switch (texture->data_type) {
        case TEXTURE_R:
            if(texture->data_depth == TEXTURE_8b)  return GL_R8;
            if(texture->data_depth == TEXTURE_16b) return GL_R16F;
            if(texture->data_depth == TEXTURE_32b) return GL_R32F;
        break;
        case TEXTURE_RG:
            if(texture->data_depth == TEXTURE_8b)  return GL_RG8;
            if(texture->data_depth == TEXTURE_16b) return GL_RG16F;
            if(texture->data_depth == TEXTURE_32b) return GL_RG32F;
        break;
        case TEXTURE_RGB:
            if(texture->data_depth == TEXTURE_16b) return GL_RGB16F;
            if(texture->data_depth == TEXTURE_32b) return GL_RGB32F;
        break;
        case TEXTURE_RGBA:
            if(texture->data_depth == TEXTURE_16b) return GL_RGBA16F;
            if(texture->data_depth == TEXTURE_32b) return GL_RGBA32F;
        break;
        case TEXTURE_DEPTH:
            if(texture->data_depth == TEXTURE_32b) return GL_DEPTH_COMPONENT32F;
        break;
    }

    // TODO(nix3l): probably shouldnt panic here
    PANIC("incorrect texture byte depth and data type combination");
    return 0;
}

texture_s create_texture_storage_type(i32 width, i32 height, texture_data_t data_type, texture_depth_t data_depth, texture_pixel_storage_t pixel_storage, void* data) {
    texture_s texture;

    texture.width = width;
    texture.height = height;

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    texture.data_type  = data_type;
    texture.data_depth = data_depth;

    texture.min_filter = TEXTURE_LINEAR;
    texture.mag_filter = TEXTURE_LINEAR;
    
    texture.wrap_mode  = TEXTURE_NO_WRAP;

    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            get_internal_format(&texture),
            width,
            height,
            0,
            texture.data_type,
            pixel_storage,
            data);

    update_texture_params(&texture);

    return texture;
}

texture_s create_texture(i32 width, i32 height, texture_data_t data_type, texture_depth_t data_depth, void* data) {
    return create_texture_storage_type(width, height, data_type, data_depth, TEXTURE_UNSIGNED_BYTE, data);
}

texture_s load_texture(char* filename) {
    texture_s texture;
    
    char* filepath = filename;

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    load_image_stb(filepath, GL_TEXTURE_2D, &texture);

    // for now just assume 16 bit floats in all textures
    texture.data_depth = TEXTURE_16b;

    texture.min_filter = TEXTURE_LINEAR;
    texture.mag_filter = TEXTURE_LINEAR;
    
    texture.wrap_mode  = TEXTURE_NO_WRAP;

    update_texture_params(&texture);

    return texture;
}

/*
texture_s create_cubemap(char** filenames, arena_s* arena) {
    texture_s texture;

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);

    for(usize i = 0; i < 6; i ++) {
        stbi_set_flip_vertically_on_load(false);

        i32 width, height, num_channels;
        unsigned char* data = stbi_load(filenames[i], &width, &height, &num_channels, 3);
        if(!data) {
            LOG_ERR("error loading cubemap texture [%s]: %s\n", filenames[i], stbi_failure_reason());
            continue;
        }

        // assume all cubemaps are just rgb
        // because honestly who even cares
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGB16F,
            width,
            height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            data);

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // NOTE(nix3l): these are order to prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texture;
}
*/

void destroy_texture(texture_s* texture) {
    glDeleteTextures(1, &texture->handle);
}

void update_texture_params(texture_s* texture) {
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->mag_filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, texture->wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap_mode);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}
