#include "debug_render.h"
#include "base_macros.h"
#include "cglm/struct/mat4.h"
#include "game/camera.h"
#include "game/game.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "render/render.h"
#include "util/math_util.h"
#include "util/util.h"

debug_render_ctx_t debug_render_ctx = {0};

static void rect_construct_uniforms(void* out, draw_call_t* call) {
    draw_pass_cache_t* cache = render_get_active_cache();

    struct  __attribute__((packed)) {
        i32 z_layer;
        mat4 model_mat;
        vec4 col;
    } uniforms = {0};

    uniforms.z_layer = 0;
    mat4s transformation = model_matrix_new(call->position, call->rotation, call->scale);
    mat4s model_mat = glms_mat4_mul(cache->proj_view, transformation);
    memcpy(uniforms.model_mat, model_mat.raw, sizeof(mat4));
    memcpy(uniforms.col, call->colour.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
}

static void circle_construct_uniforms(void* out, draw_call_t* call) {
    draw_pass_cache_t* cache = render_get_active_cache();

    struct  __attribute__((packed)) {
        i32 z_layer;
        mat4 model_mat;
        vec4 col;
    } uniforms = {0};

    uniforms.z_layer = 0;
    mat4s transformation = model_matrix_new(call->position, call->rotation, call->scale);
    memcpy(uniforms.model_mat, glms_mat4_mul(cache->proj_view, transformation).raw, sizeof(mat4));
    memcpy(uniforms.col, call->colour.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
}

void debug_render_init() {
    // yeah im leaking memory sue me
    arena_t arena = arena_alloc_new(4096);

    range_t rect_vs = platform_load_file(&arena, "shader/debug/rect.vs");
    range_t rect_fs = platform_load_file(&arena, "shader/debug/rect.fs");
    shader_t rect_shader = shader_new((shader_info_t) {
        .name = "rect-shader",
        .attribs = {
            { .name = "vs_position" },
        },
        .uniforms = {
             { .name = "z_layer",   .type = UNIFORM_TYPE_i32, },
             { .name = "model_mat", .type = UNIFORM_TYPE_mat4, },
             { .name = "col",       .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = rect_vs,
        .fragment_src = rect_fs,
    });

    range_t circle_vs = platform_load_file(&arena, "shader/debug/circle.vs");
    range_t circle_fs = platform_load_file(&arena, "shader/debug/circle.fs");
    shader_t circle_shader = shader_new((shader_info_t) {
        .name = "circle-shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
             { .name = "z_layer",   .type = UNIFORM_TYPE_i32, },
             { .name = "model_mat", .type = UNIFORM_TYPE_mat4, },
             { .name = "col",       .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = circle_vs,
        .fragment_src = circle_fs,
    });

    renderer_t renderer = (renderer_t) {
        .label = "debug-renderer",
        .num_groups = 2,
        .groups = {
            [0] = {
                .pass = {
                    .label = "rect-pass",
                    .type = DRAW_PASS_RENDER,
                    .pipeline = {
                        .cull.enable = true,
                        .shader = rect_shader,
                    },
                    .state = {
                        .anchor.enable = true,
                        .projection.type = PROJECTION_ORTHO,
                    },
                },
                .batch = llist_new(),
                .construct_uniforms = rect_construct_uniforms,
            },
            [1] = {
                .pass = {
                    .label = "circle-pass",
                    .type = DRAW_PASS_RENDER,
                    .pipeline = {
                        .cull.enable = true,
                        .shader = circle_shader,
                    },
                    .state = {
                        .anchor.enable = true,
                        .projection.type = PROJECTION_ORTHO,
                    },
                },
                .batch = llist_new(),
                .construct_uniforms = circle_construct_uniforms,
            },
        },
    };

    debug_render_ctx = (debug_render_ctx_t) {
        .renderer = renderer,
    };
}

void debug_render_terminate() {
    // do stuff
}

static v4f debug_render_color(debug_render_color_t col) {
    switch (col) {
        // gruvbox colors
        case DEBUG_COLOR_RED:    return v4f_rgba(219.0f, 73.0f,  52.0f,  1.0f);
        case DEBUG_COLOR_ORANGE: return v4f_rgba(254.0f, 128.0f, 25.0f,  1.0f);
        case DEBUG_COLOR_YELLOW: return v4f_rgba(250.0f, 189.0f, 47.0f,  1.0f);
        case DEBUG_COLOR_GREEN:  return v4f_rgba(184.0f, 187.0f, 38.0f,  1.0f);
        case DEBUG_COLOR_BLUE:   return v4f_rgba(131.0f, 165.0f, 152.0f, 1.0f);
        case DEBUG_COLOR_AQUA:   return v4f_rgba(142.0f, 192.0f, 124.0f, 1.0f);
        case DEBUG_COLOR_PURPLE: return v4f_rgba(211.0f, 134.0f, 155.0f, 1.0f);
        case DEBUG_COLOR_BLACK:  return v4f_rgba(29.0f,  32.0f,  33.0f,  1.0f);
        case DEBUG_COLOR_WHITE:  return v4f_rgba(251.0f, 241.0f, 199.0f, 1.0f);
        // TODP(nix3l): this is stupid
        case DEBUG_COLOR_RANDOM: return debug_render_color(rand() % DEBUG_COLOR_RANDOM);
        default: UNREACHABLE; return v4f_ONE;
    }
}

static void debug_render_push_rect(debug_render_call_t call) {
    render_push_draw_call(&debug_render_ctx.renderer.groups[0], (draw_call_t) {
        .position = v3f_new(call.data.rect.centre.x, call.data.rect.centre.y, 0.0f),
        .rotation = v3f_new(0.0f, 0.0f, call.data.rect.rot),
        .scale = v3f_new(call.data.rect.size.x, call.data.rect.size.y, 1.0f),
        .colour = debug_render_color(call.color),
    });
}

static void debug_render_push_circle(debug_render_call_t call) {
    render_push_draw_call(&debug_render_ctx.renderer.groups[1], (draw_call_t) {
        .position = v3f_new(call.data.circle.centre.x, call.data.circle.centre.y, 0.0f),
        .scale = v3f_new(call.data.circle.radius, call.data.circle.radius, 1.0f),
        .colour = debug_render_color(call.color),
    });
}

void debug_render(debug_render_call_t call) {
    switch(call.shape) {
        case DEBUG_SHAPE_INVALID: LOG_ERR("invalid debug render call shape"); break;
        case DEBUG_SHAPE_RECT: debug_render_push_rect(call); break;
        case DEBUG_SHAPE_CIRCLE:  debug_render_push_circle(call); break;
    }
}

void debug_render_dispatch() {
    for(u32 i = 0; i < debug_render_ctx.renderer.num_groups; i ++) {
        camera_attach(&game_ctx.camera, &debug_render_ctx.renderer.groups[i].pass);
    }

    render_dispatch(&debug_render_ctx.renderer);
}
