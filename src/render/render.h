#ifndef _RENDER_H
#define _RENDER_H

#include "base.h"
#include "gfx/gfx.h"

enum {
    RENDER_MAX_CALLS = 4096,
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
    u32 x, y, w, h;
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

typedef struct draw_pass_t {
    const char* label;
    draw_pass_type_t type;
    render_pipeline_t pipeline;
    draw_pass_state_t state;
} draw_pass_t;

void render_activate_pass(draw_pass_t pass);
void render_clear_active_pass();

// the omega draw call struct
// put ALL the things in here (dont be shy)
// until memory becomes an issue everything goes in here
typedef struct draw_call_t {
    v3f position;
    v3f rotation;
    v3f scale;
    v4f colour;
} draw_call_t;

// RENDERER
typedef struct renderer_t {
    const char* label;
    vector_t batch;
    draw_pass_t pass;
} renderer_t;

void render_push_draw_call(draw_call_t call);
void render_dispatch();

// CONTEXT
typedef struct render_ctx_t {
    mesh_t unit_square;

    renderer_t renderer;

    draw_pass_t active_pass;
} render_ctx_t;

extern render_ctx_t render_ctx;

void render_init();
void render_terminate();

#endif
