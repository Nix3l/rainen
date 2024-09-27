#ifndef TEXTURE_H
#define TEXTURE_H

#include "base.h"
#include "memory/memory.h"

typedef struct {
    i32 width, height;

    GLenum internal_format, data_format;

    GLenum min_filter, mag_filter;

    GLuint id;
} texture_s;

typedef struct {
    i32 width, height, depth;

    GLenum internal_format, data_format;

    GLuint id;
} texture_3d_s;

// TODO(nix3l): create_texture_format and create_cubemap_format
texture_s create_texture(char* filename, arena_s* arena);
texture_3d_s create_texture_3d(i32 width, i32 height, i32 depth, void* data);
texture_3d_s create_texture_3d_format(i32 width, i32 height, i32 depth, GLenum internal_format, GLenum data_format, void* data);
// ordered: right, left, top, bottom, front, back
texture_s create_cubemap(char** filenames, arena_s* arena);

void destroy_texture(texture_s* texture);
void destroy_texture_3d(texture_3d_s* texture);

#endif
