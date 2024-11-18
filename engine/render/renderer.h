#ifndef RENDERER_H
#define RENDERER_H

#include "base.h"
#include "util/vector.h"
#include "shader/shader.h"
#include "texture/texture.h"
#include "camera/camera.h"
#include "framebuffer/fbo.h"
#include "mesh/mesh.h"
#include "font/font.h"

#define MAX_DRAW_GROUPS 32
#define MAX_DRAW_CALLS  4096

// TODO(nix3l): user data to pass to uniforms
typedef struct {
    camera_s* camera;

    mesh_s* mesh;
    texture_s* texture;

    v2f position;
    mat4s transformation;
    f32 stroke_size;

    i32 layer;

    v4f color;
} draw_call_s;

typedef enum {
    DRAW_GROUP_NONE = 0x0,
    DRAW_GROUP_DEPTH_TEST = 0x01,
    DRAW_GROUP_CULL = 0x02,
} draw_group_tags_t;

typedef struct {
    shader_s* shader;
    camera_s* camera;
    fbo_s* framebuffer;

    mesh_s* fallback_mesh;

    // TAGS
    draw_group_tags_t tags;
    GLenum depth_mask, depth_func;
    GLenum cull_face;

    projection_e projection_type;

    // CALLS
    u32 num_calls;
    arena_s draw_calls;
} draw_group_s;

typedef struct {
    u32 num_groups;
    arena_s* groups;

    fbo_s* screen_buffer;
} renderer_s;

void init_renderer(renderer_s* renderer, arena_s* arena, fbo_s* screen);

draw_group_s* push_draw_group(renderer_s* renderer, shader_s* shader, camera_s* camera);
// TODO(nix3l): change to take in just a transformation matrix
draw_call_s* push_draw_call(draw_group_s* group, mesh_s* mesh, texture_s* texture, v2f position, f32 rotation, v2f scale, f32 stroke_size, i32 layer, v4f color);
draw_call_s* push_text_draw_call(draw_group_s* text_group, font_s* font, i32 size, char* text, u32 text_length, v2f start, arena_s* arena);

void render_draw_group(draw_group_s* group);
void render_draw_groups(renderer_s* renderer);

void flush_draw_group(draw_group_s* draw_group);

#endif
