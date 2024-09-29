#ifndef RENDERER_H
#define RENDERER_H

#include "base.h"
#include "util/vector.h"
#include "shader/shader.h"
#include "texture/texture.h"
#include "camera/camera.h"
#include "framebuffer/fbo.h"

#define MAX_DRAW_GROUPS 32
#define MAX_DRAW_CALLS  4096

typedef struct {
    texture_s* texture;

    v2f position;
    mat4s transformation;

    i32 layer;

    v4f color;
} draw_call_s;

typedef struct {
    shader_s* shader;
    camera_s* camera;

    fbo_s* framebuffer;

    bool enable_depth_test;
    GLenum depth_mask, depth_func;

    bool enable_culling;
    GLenum cull_face;

    projection_e projection_type;

    u32 num_calls;
    arena_s draw_calls;
} draw_group_s;

typedef struct {
    u32 num_groups;
    arena_s* groups;
} renderer_s;

void init_renderer(renderer_s* renderer, arena_s* arena);

draw_call_s* push_draw_call(draw_group_s* group, texture_s* texture, v2f position, i32 layer, v4f colors);
draw_call_s* push_draw_call_transformed(draw_group_s* group, texture_s* texture, v2f position, f32 rotation, v2f scale, i32 layer, v4f color);
draw_group_s* push_draw_group(renderer_s* renderer, shader_s* shader, camera_s* camera);

void render_draw_call(draw_call_s* call, shader_s* shader, camera_s* camera);
void render_draw_group(draw_group_s* group);
void render_draw_groups(renderer_s* renderer);

void flush_draw_group(draw_group_s* draw_group);

#endif
