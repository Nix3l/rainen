#ifndef FBO_H
#define FBO_H

#include "base.h"
#include "memory/memory.h"
#include "texture/texture.h"

typedef struct {
    GLuint handle;

    i32 width, height;

    u32 num_textures; // does NOT count the depth texture
    texture_s* textures;

    texture_s depth;

    GLenum* attachments;
} fbo_s;

fbo_s create_fbo(u32 width, u32 height, u32 num_textures, arena_s* arena);
void destroy_fbo(fbo_s* fbo);

void fbo_create_texture(fbo_s* fbo, GLenum attachment_type, texture_data_t data_type, texture_depth_t data_depth);
void fbo_create_depth_texture(fbo_s* fbo);

void fbo_clear(fbo_s* fbo, v3f col, GLbitfield clear_bit);

void fbo_copy_texture_to_screen(fbo_s* fbo, GLenum src_att);
void fbo_copy_texture(fbo_s* src_fbo, fbo_s* dest_fbo, GLenum src_att);
void fbo_copy_depth_texture(fbo_s* src, fbo_s* dest);

#endif
