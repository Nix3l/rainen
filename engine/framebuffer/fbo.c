#include "fbo.h"

#include "engine.h"
#include "util/log.h"
#include "memory/memory.h"

fbo_s create_fbo(u32 width, u32 height, u32 num_textures, arena_s* arena) {
    fbo_s fbo;

    glGenFramebuffers(1, &fbo.handle);

    fbo.width = width;
    fbo.height = height;
    fbo.num_textures = num_textures;

    fbo.textures = arena_push(arena, num_textures * sizeof(texture_s));
    MEM_ZERO(fbo.textures, num_textures * sizeof(texture_s));

    fbo.attachments = arena_push(arena, num_textures * sizeof(GLenum));

    return fbo;
}

void destroy_fbo(fbo_s* fbo) {
    for(usize i = 0; i < fbo->num_textures; i ++)
        glDeleteTextures(1, &fbo->textures[i].handle);

    glDeleteFramebuffers(1, &fbo->handle);
}

void fbo_create_texture(fbo_s* fbo, GLenum attachment_type, texture_data_t data_type, texture_depth_t data_depth) {
    texture_s* texture = NULL;
    
    // grab the first texture that has not been initialised
    for(usize i = 0; i < fbo->num_textures; i ++) {
        if(fbo->textures[i].handle == 0) {
            texture = &fbo->textures[i];

            // not the biggest fan of putting this here
            // because it doesnt make sense but hey it works
            fbo->attachments[i] = attachment_type;
            break;
        }
    }

    if(!texture) {
        LOG_ERR("not enough space in fbo for another texture! num_textures: [%u]\n", fbo->num_textures);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle);

    *texture = create_texture(fbo->width, fbo->height, data_type, data_depth, NULL);

    glBindTexture(GL_TEXTURE_2D, texture->handle);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, texture->handle, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fbo_create_depth_texture(fbo_s* fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle);

    fbo->depth = create_texture_storage_type(fbo->width, fbo->height, TEXTURE_DEPTH, TEXTURE_32b, TEXTURE_FLOAT, NULL);

    fbo->depth.min_filter = TEXTURE_NEAREST;
    fbo->depth.mag_filter = TEXTURE_NEAREST;
    update_texture_params(&fbo->depth);

    glBindTexture(GL_TEXTURE_2D, fbo->depth.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE,   GL_ALPHA);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->depth.handle, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fbo_clear(fbo_s* fbo, v3f col, GLbitfield clear_bit) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle);
    glClearColor(col.r, col.g, col.b, 1.0f);
    glClear(clear_bit);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fbo_copy_texture_to_screen(fbo_s* fbo, GLenum src_att) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle);

    glReadBuffer(src_att);

    glBlitFramebuffer(0, 0, fbo->width, fbo->height, 
                      0, 0, engine_state->window.width, engine_state->window.height,
                      GL_COLOR_BUFFER_BIT,
                      GL_LINEAR);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void fbo_copy_texture(fbo_s* src, fbo_s* dest, GLenum src_att) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->handle);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->handle);

    glReadBuffer(src_att);

    glBlitFramebuffer(0, 0, src->width, src->height, 
                      0, 0, dest->width, dest->height,
                      GL_COLOR_BUFFER_BIT,
                      GL_LINEAR);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void fbo_copy_depth_texture(fbo_s* src, fbo_s* dest) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->handle);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->handle);

    glReadBuffer(GL_DEPTH_ATTACHMENT);

    glBlitFramebuffer(0, 0, src->width, src->height, 
                      0, 0, dest->width, dest->height,
                      GL_DEPTH_BUFFER_BIT,
                      GL_LINEAR);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
