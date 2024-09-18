#include "texture.h"
#include "util/log.h"
#include "platform/platform.h"

static void load_image_stb(char* filepath, GLenum target, texture_s* texture) {
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

    if(texture) {
        texture->width = width;
        texture->height = width;

        texture->internal_format = internal_format;
        texture->data_format = format;
    }

    stbi_image_free(data);
}

texture_s create_texture(char* filename, arena_s* arena) {
    texture_s texture;
    
    char* filepath = platform_get_res_path(filename, arena);

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    load_image_stb(filepath, GL_TEXTURE_2D, &texture);

    texture.full_path = filepath;
    // NOTE(nix3l): assuming that the file name is in the arena already
    texture.name = filename;

    // TODO(nix3l): parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

texture_3d_s create_texture_3d(i32 width, i32 height, i32 depth, void* data) {
    texture_3d_s texture;

    texture.name = NULL;

    texture.width  = width;
    texture.height = height;
    texture.depth  = depth;
    
    texture.internal_format = GL_RGB;
    texture.data_format = GL_RGB16F;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_3D, texture.id);

    glTexImage3D(
            GL_TEXTURE_3D,
            0,
            texture.internal_format,
            width,
            height,
            depth,
            0,
            texture.data_format,
            GL_UNSIGNED_BYTE,
            data);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);
    return texture;
}

texture_3d_s create_texture_3d_format(i32 width, i32 height, i32 depth, GLenum internal_format, GLenum data_format, void* data) {
    texture_3d_s texture;

    texture.name = NULL;

    texture.width  = width;
    texture.height = height;
    texture.depth  = depth;
    
    texture.internal_format = internal_format;
    texture.data_format     = data_format;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_3D, texture.id);

    glTexImage3D(
            GL_TEXTURE_3D,
            0,
            texture.internal_format,
            width,
            height,
            depth,
            0,
            texture.data_format,
            GL_UNSIGNED_BYTE,
            data);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /*
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    */

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glBindTexture(GL_TEXTURE_3D, 0);
    return texture;
}

texture_s create_cubemap(char** filenames, arena_s* arena) {
    texture_s texture;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.id);

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
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    texture.full_path = NULL;
    texture.name = NULL;

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texture;
}

void destroy_texture(texture_s* texture) {
    glDeleteTextures(1, &texture->id);
}

void destroy_texture_3d(texture_3d_s* texture) {
    glDeleteTextures(1, &texture->id);
}
