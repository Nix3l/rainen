#ifndef _RENDER_H
#define _RENDER_H

#include "base.h"
#include "gfx/gfx.h"

// TODO(nix3l): move viewport to pipeline?

enum {
    RENDER_MAX_CALLS = 4096,
    RENDER_MAX_GROUPS = 8,
};

// DRAW PARAMETERS
typedef enum projection_type_t {
    PROJECTION_NONE = 0,
    PROJECTION_PERSPECTIVE,
    PROJECTION_ORTHO,
} projection_type_t;

typedef struct draw_projection_t {
    projection_type_t type;
    f32 fov;
    f32 aspect_ratio;
    f32 near, far;
    f32 w, h;
} draw_projection_t;

typedef struct draw_anchor_t {
    bool enable;
    v3f position;
    v3f rotation;
} draw_anchor_t;

typedef struct draw_viewport_t {
    bool enable;
    viewport_t viewport;
} draw_viewport_t;

// DRAW PASS
typedef enum draw_pass_type_t {
    DRAW_PASS_INVALID = 0,
    DRAW_PASS_RENDER,
    // DRAW_PASS_RENDER_INSTANCED,
    // DRAW_PASS_POSTPROCESS,
    // DRAW_PASS_COMPUTE,
} draw_pass_type_t;

typedef struct draw_pass_state_t {
    draw_anchor_t anchor;
    draw_projection_t projection;
    draw_viewport_t viewport;
} draw_pass_state_t;

typedef struct draw_pass_cache_t {
    mat4s projView;
} draw_pass_cache_t;

typedef struct draw_pass_t {
    const char* label;
    draw_pass_type_t type;
    render_pipeline_t pipeline;
    draw_pass_state_t state;
    draw_pass_cache_t cache;
} draw_pass_t;

// the omega draw call struct
// put ALL the things in here (dont be shy)
// until memory becomes an issue everything goes in here
typedef struct draw_call_t {
    v3f position;
    v3f rotation;
    v3f scale;
    v2f min;
    v2f max;
    v4f colour;
    v4f bg;
    f32 stroke;
    sampler_slot_t sampler;
} draw_call_t;

typedef struct draw_group_t {
    vector_t batch;
    draw_pass_t pass;
    void (*construct_uniforms) (void* out, draw_call_t* call);
} draw_group_t;

void render_activate_group(draw_group_t group);
void render_clear_active_group();
void render_push_draw_call(draw_group_t* group, draw_call_t call);

// RENDERER
typedef struct renderer_t {
    const char* label;
    u32 num_groups;
    draw_group_t groups[RENDER_MAX_GROUPS];
} renderer_t;

void render_dispatch(renderer_t* renderer);

// CONTEXT
typedef struct render_ctx_t {
    mesh_t unit_square;

    draw_group_t active_group;

    renderer_t renderer;
} render_ctx_t;

extern render_ctx_t render_ctx;

void render_init();
void render_terminate();

#endif
