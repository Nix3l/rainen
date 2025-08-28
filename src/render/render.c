#include "render.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "util/util.h"
#include "util/math_util.h"
#include "errors/errors.h"

render_ctx_t render_ctx;

void render_init() {
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
        .active_group = {0},
    };
}

void render_terminate() {
    // do stuff
}

void render_activate_group(draw_group_t group) {
    if(group.pass.type == DRAW_PASS_INVALID) {
        LOG_ERR_CODE(ERR_RENDER_BAD_PASS);
        return;
    }

    render_ctx.active_group = group;

    gfx_activate_pipeline(group.pass.pipeline);
}

void render_clear_active_group() {
    render_ctx.active_group = (draw_group_t) {0};
}

void render_push_draw_call(draw_group_t* group, draw_call_t call) {
    if(group->batch.size == group->batch.capacity) {
        LOG_ERR_CODE(ERR_RENDER_CALL_LIMIT_REACHED);
        return;
    }

    vector_push_data(&group->batch, &call);
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

static void group_update_cache() {
    if(render_ctx.active_group.pass.type == DRAW_PASS_INVALID) {
        LOG_ERR_CODE(ERR_RENDER_NO_ACTIVE_GROUP);
        return;
    }

    render_ctx.active_group.pass.cache = (draw_pass_cache_t) {
        .projView = pass_get_proj_view(render_ctx.active_group.pass),
    };
}

static void render_active_group() {
    draw_group_t group = render_ctx.active_group;
    draw_pass_t pass = group.pass;
    if(pass.type == DRAW_PASS_INVALID) {
        LOG_ERR_CODE(ERR_RENDER_NO_ACTIVE_GROUP);
        return;
    }

    range_t uniforms = range_alloc_new(shader_get_uniforms_size(pass.pipeline.shader));

    for(u32 i = 0; i < group.batch.size; i ++) {
        draw_call_t* call = vector_get(&group.batch, i);
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

        group.construct_uniforms(uniforms.ptr, call);

        shader_update_uniforms(pass.pipeline.shader, uniforms);
        gfx_draw();

        mem_clear(uniforms.ptr, uniforms.size);
    }

    range_destroy(&uniforms);
}

void render_dispatch(renderer_t* renderer) {
    for(u32 i = 0; i < renderer->num_groups; i ++) {
        render_activate_group(renderer->groups[i]);
        group_update_cache();
        render_active_group();
        vector_clear(&renderer->groups[i].batch);
        render_clear_active_group();
        gfx_clear_active_pipeline();
    }
}
