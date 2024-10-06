#ifndef TEXTURE_H
#define TEXTURE_H

#include "base.h"
#include "memory/memory.h"

typedef enum {
    TEXTURE_16b = 1,
    TEXTURE_32b = 2,
} texture_depth_e;

typedef enum {
    TEXTURE_R     = GL_RED,
    TEXTURE_RG    = GL_RG,
    TEXTURE_RGB   = GL_RGB,
    TEXTURE_RGBA  = GL_RGBA,
    TEXTURE_DEPTH = GL_DEPTH_COMPONENT,
} texture_data_e;

typedef enum {
    TEXTURE_UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    TEXTURE_FLOAT         = GL_FLOAT,
} texture_pixel_storage_e;

typedef enum {
    TEXTURE_NO_WRAP         = GL_CLAMP_TO_EDGE,
    TEXTURE_REPEAT          = GL_REPEAT,
    TEXTURE_MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
} texture_wrap_e;

typedef enum {
    TEXTURE_NEAREST = GL_NEAREST,
    TEXTURE_LINEAR  = GL_LINEAR,
} texture_filter_e;

typedef struct {
    GLuint handle;

    i32 width, height;

    i32 num_channels;
    texture_data_e data_type;
    texture_depth_e data_depth;
    // NOTE(nix3l): currently unnecessary to be stored
    // texture_pixel_storage_e pixel_storage;
    texture_filter_e min_filter, mag_filter;
    texture_wrap_e wrap_mode;
} texture_s;

texture_s create_texture_storage_type(i32 width, i32 height, texture_data_e data_type, texture_depth_e data_depth, texture_pixel_storage_e pixel_storage);
texture_s create_texture(i32 width, i32 height, texture_data_e data_type, texture_depth_e data_depth);
texture_s load_texture(char* filename);
void destroy_texture(texture_s* texture);

void update_texture_params(texture_s* texture);

#endif
