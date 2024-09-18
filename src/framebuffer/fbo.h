#ifndef FBO_H
#define FBO_H

#include "base.h"
#include "memory/memory.h"

typedef struct {
    GLuint id;
    GLenum attachment;

    GLint internal_format;
    GLenum format;
} fbo_texture_s;

typedef struct {
    GLuint id;

    u32 width;
    u32 height;

    usize num_textures; // does NOT count the depth texture
    fbo_texture_s* textures;

    fbo_texture_s depth;

    GLenum* attachments;
} fbo_s;

fbo_s create_fbo(u32 width, u32 height, usize num_textures, arena_s* arena);
void destroy_fbo(fbo_s* fbo);

void fbo_create_texture(fbo_s* fbo, GLenum attachment_type, GLint internal_format, GLenum format);
void fbo_create_depth_texture(fbo_s* fbo);

void fbo_clear(fbo_s* fbo, v3f col, GLbitfield clear_bit);

void fbo_copy_texture_to_screen(fbo_s* fbo, GLenum src_att);
void fbo_copy_texture(fbo_s* src_fbo, fbo_s* dest_fbo, GLenum src_att);
void fbo_copy_depth_texture(fbo_s* src, fbo_s* dest);

#endif
