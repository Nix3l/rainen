#include "game.h"
#include "entity.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "physics/bounds.h"
#include "platform/platform.h"
#include "render/render.h"
#include "util/math_util.h"
#include "io/io.h"
#include "rations/rations.h"

game_ctx_t game_ctx;

static void construct_uniforms(void* out, draw_call_t* call) {
    draw_pass_cache_t cache = render_ctx.active_group.pass.cache;

    struct __attribute__((packed)) {
        mat4 projViewModel;
        vec4 col;
    } uniforms;

    mat4s projViewModel = glms_mat4_mul(cache.projView, model_matrix_new(call->position, call->rotation, call->scale));
    memcpy(uniforms.projViewModel, projViewModel.raw, sizeof(mat4));

    memcpy(uniforms.col, call->colour.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
}

void game_init() {
    arena_t game_rations = arena_new(rations.game);

    arena_t code_arena = arena_alloc_new(4096);
    range_t shader_vs = platform_load_file(&code_arena, "shader/default.vs");
    range_t shader_fs = platform_load_file(&code_arena, "shader/default.fs");
    shader_t shader = shader_new((shader_info_t) {
        .name = "game shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
            { .name = "projViewModel", .type = UNIFORM_TYPE_mat4, },
            { .name = "col", .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = shader_vs,
        .fragment_src = shader_fs,
    });

    renderer_t renderer = (renderer_t) {
        .label = "entity renderer",
        .num_groups = 1,
        .groups = {
            [0] = {
                .pass = {
                    .label = "_",
                    .type = DRAW_PASS_RENDER,
                    .pipeline = {
                        .clear = { .colour = true, .depth = true, },
                        .cull = { .enable = true, },
                        .blend = {
                            .enable = true,
                            .dst_func = BLEND_FUNC_SRC_ONE_MINUS_ALPHA,
                            .src_func = BLEND_FUNC_SRC_ALPHA,
                        },
                        .depth = { .enable = true, },
                        .shader = shader,
                    },
                    .state = {
                        .anchor = { .enable = true, },
                        .projection = { .type = PROJECTION_ORTHO, },
                    },
                },
                .batch = llist_new(),
                .construct_uniforms = construct_uniforms,
            },
        },
    };

    camera_t camera = (camera_t) {
        .transform = (transform_t) {
            .position = v2f_ZERO,
            .rotation = 0.0f,
            .z = 100,
        },
        .near = 0.1f,
        .far = 200.0f,
        .pixel_scale = 1.0f,
    };

    game_ctx = (game_ctx_t) {
        .rations = game_rations,
        .camera = camera,
        .renderer = renderer,
    };
}

void game_terminate() {
    arena_clear(&game_ctx.rations);
}

void game_load_room(room_t room) {
    game_ctx.room = room;
    game_ctx.camera.transform.position = room.camvol.center;
    game_ctx.camera.w = room.camvol.dimensions.x;
    game_ctx.camera.h = room.camvol.dimensions.y;
}

void game_update() {
    entity_update();
}

void game_render() {
    room_render(&game_ctx.room, &game_ctx.renderer.groups[0]);

    for(u32 i = 0; i < game_ctx.renderer.num_groups; i ++) {
        camera_attach(&game_ctx.camera, &game_ctx.renderer.groups[i].pass);
    }

    render_dispatch(&game_ctx.renderer);
}
