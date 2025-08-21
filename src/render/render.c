#include "render.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "util/util.h"
#include "util/math_util.h"
#include "errors/errors.h"

render_ctx_t render_ctx;

static void renderer_construct_uniforms(void* out, draw_call_t* call) {
    draw_pass_cache_t cache = render_ctx.active_pass.cache;

    struct {
        mat4 projViewModel;
        vec4 col;
    } uniforms;

    mat4s modelViewProj = glms_mat4_mul(cache.projView, model_matrix_new(call->position, call->rotation, call->scale));
    glm_mat4_copy(modelViewProj.raw, uniforms.projViewModel);

    memcpy(uniforms.col, call->colour.raw, sizeof(vec4));

    memcpy(out, &uniforms, sizeof(uniforms));
}

void render_init() {
    // leak ALL the memory
    arena_t shader_code_arena = arena_alloc_new(4096, EXPAND_TYPE_IMMUTABLE);
    range_t vertex_src = platform_load_file(&shader_code_arena, "shader/default.vs");
    range_t fragment_src = platform_load_file(&shader_code_arena, "shader/default.fs");
    shader_t shader = shader_new((shader_info_t) {
        .name = "shader",
        .pretty_name = "shader",
        .attribs = {
            { .name = "vs_position" },
            { .name = "vs_uvs" },
        },
        .uniforms = {
            { .name = "projViewModel", .type = UNIFORM_TYPE_mat4, },
            { .name = "col", .type = UNIFORM_TYPE_v4f, },
        },
        .vertex_src = vertex_src,
        .fragment_src = fragment_src,
    });

    renderer_t renderer = {
        .label = "renderer",
        .pass = {
            .label = "pass",
            .type = DRAW_PASS_RENDER,
            .pipeline = {
                .clear = {
                    .depth = true,
                    .colour = true,
                    .clear_col = v4f_new(0.1f, 0.1f, 0.1f, 1.0f),
                },
                .cull = { .enable = true, },
                .depth = { .enable = true, },
                .shader = shader,
            },
            .state = {
                .anchor = { .enable = true, .position = v3f_new(0.0f, 0.0f, 100.0f), },
                .projection = {
                    .type = PROJECTION_ORTHO,
                    .fov = RADIANS(80.0f),
                    .aspect_ratio = 16.0f/9.0f,
                    .w = 1600.0f,
                    .h = 900.0f,
                    .near = 0.001f,
                    .far = 1000.0f,
                },
            },
        },
        .batch = vector_alloc_new(RENDER_MAX_CALLS, sizeof(draw_call_t)),
        .construct_uniforms = renderer_construct_uniforms,
    };

    f32 vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    f32 uvs[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    u32 indices[] = {
        0, 1, 2,
        2, 1, 3,
    };

    mesh_t mesh = mesh_new((mesh_info_t) {
        .format = MESH_FORMAT_X2T2,
        .index_type = MESH_INDEX_32b,
        .indices = range_new(indices, sizeof(indices)),
        .attributes = {
            mesh_attribute(vertices, sizeof(vertices), 2),
            mesh_attribute(uvs, sizeof(uvs), 2),
        },
        .count = 6,
    });

    render_ctx = (render_ctx_t) {
        .unit_square = mesh,
        .active_pass = {0},
        .renderer = renderer,
    };
}

void render_terminate() {
    shader_destroy(render_ctx.renderer.pass.pipeline.shader);
}

void render_activate_pass(draw_pass_t pass) {
    if(pass.type == DRAW_PASS_INVALID) {
        LOG_ERR_CODE(ERR_RENDER_BAD_PASS);
        return;
    }

    render_ctx.active_pass = pass;

    gfx_activate_pipeline(pass.pipeline);
}

void render_clear_active_pass() {
    render_ctx.active_pass = (draw_pass_t) {0};
}

static mat4s pass_get_proj_view(draw_pass_t pass) {
    draw_anchor_t anchor = pass.state.anchor;
    draw_projection_t proj = pass.state.projection;

    mat4s projView = mat4_IDENTITY;

    if(proj.type == PROJECTION_PERSPECTIVE)
        projView = glms_mat4_mul(projView, proj_perspective_matrix_new(proj.fov, proj.aspect_ratio, proj.near, proj.far));
    else if(proj.type == PROJECTION_ORTHO)
        projView = glms_mat4_mul(projView, proj_ortho_matrix_new(proj.w, proj.h, proj.near, proj.far));

    if(anchor.enable)
        projView = glms_mat4_mul(projView, view_matrix_new(anchor.position, anchor.rotation));

    return projView;
}

static void pass_update_cache() {
    render_ctx.active_pass.cache = (draw_pass_cache_t) {
        .projView = pass_get_proj_view(render_ctx.active_pass),
    };
}

void render_push_draw_call(renderer_t* renderer, draw_call_t call) {
    vector_push_data(&renderer->batch, &call);
}

static void render_batch(renderer_t* renderer) {
    draw_pass_t pass = render_ctx.active_pass;
    if(pass.type == DRAW_PASS_INVALID) {
        LOG_ERR_CODE(ERR_RENDER_NO_ACTIVE_PASS);
        return;
    }

    range_t uniforms = range_alloc(shader_get_uniforms_size(pass.pipeline.shader));

    for(u32 i = 0; i < renderer->batch.size; i ++) {
        draw_call_t* call = vector_get(&renderer->batch, i);
        if(!call) {
            LOG_ERR_CODE(ERR_RENDER_BAD_CALL);
            UNREACHABLE; // bit harsh but just to make sure in dev
        }

        gfx_supply_bindings((render_bindings_t) {
            .mesh = render_ctx.unit_square,
            .texture_samplers = {
                [0] = call->sampler,
            },
        });

        renderer->construct_uniforms(uniforms.ptr, call);

        shader_update_uniforms(pass.pipeline.shader, uniforms);
        gfx_draw();

        mem_clear(uniforms.ptr, uniforms.size);
    }
}

void render_dispatch(renderer_t* renderer) {
    render_activate_pass(renderer->pass);
    pass_update_cache();
    render_batch(renderer);
    vector_clear(&renderer->batch);
    render_clear_active_pass();
}
