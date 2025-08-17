#include "render.h"
#include "platform/platform.h"
#include "util/util.h"
#include "util/math_util.h"
#include "errors/errors.h"

render_ctx_t render_ctx;

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
                .anchor = { .enable = true, .position = v3f_new(0.0f, 0.0f, 2.0f), },
                .projection = {
                    .type = PROJECTION_PERSPECTIVE,
                    .fov = RADIANS(80.0f),
                    .aspect_ratio = 16.0f/9.0f,
                    .w = 16.0f,
                    .h = 9.0f,
                    .near = 0.001f,
                    .far = 1000.0f,
                },
            },
        },
        .batch = vector_alloc_new(RENDER_MAX_CALLS, sizeof(draw_call_t)),
    };

    f32 vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    f32 uvs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
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
        .renderer = renderer,
        .active_pass = {0},
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

void render_push_draw_call(draw_call_t call) {
    vector_push_data(&render_ctx.renderer.batch, &call);
}

static void render_flush() {
    vector_clear(&render_ctx.renderer.batch);
}

void render_dispatch() {
    render_activate_pass(render_ctx.renderer.pass);

    draw_pass_t pass = render_ctx.active_pass;
    mat4s projView = pass_get_proj_view(pass);

    for(u32 i = 0; i < render_ctx.renderer.batch.size; i ++) {
        draw_call_t* call = vector_get(&render_ctx.renderer.batch, i);
        if(!call) {
            LOG_ERR_CODE(ERR_RENDER_BAD_CALL);
            UNREACHABLE; // bit harsh but just to make sure in dev
        }

        struct {
            mat4 projViewModel;
            vec4 col;
        } uniforms;

        gfx_supply_bindings((render_bindings_t) {
            .mesh = render_ctx.unit_square,
        });

        mat4s modelViewProj = glms_mat4_mul(projView, model_matrix_new(call->position, call->rotation, call->scale));
        glm_mat4_copy(modelViewProj.raw, uniforms.projViewModel);

        memcpy(uniforms.col, call->colour.raw, sizeof(vec4));

        shader_update_uniforms(pass.pipeline.shader, range_new(&uniforms, sizeof(uniforms)));
        gfx_draw();
    }

    render_clear_active_pass();

    render_flush();
}
