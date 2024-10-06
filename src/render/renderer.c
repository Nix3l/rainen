#include "renderer.h"
#include "game.h"
#include "util/log.h"

void init_renderer(renderer_s* renderer, arena_s* arena, fbo_s* screen) {
    *renderer = (renderer_s) {
        .num_groups = 0,
        .groups = arena,

        .screen_buffer = screen,
        .unit_mesh = primitive_unit_square(),
    };
}

draw_call_s* push_draw_call(draw_group_s* group, texture_s* texture, v2f position, i32 layer, v4f color) {
    if(group->num_calls + 1 > MAX_DRAW_CALLS) {
        LOG_ERR("reached max draw calls in group\n");
        return NULL;
    }

    group->num_calls ++;
    draw_call_s* call = arena_push(&group->draw_calls, sizeof(draw_call_s));

    call->texture = texture;
    call->position = position;
    call->transformation = glms_translate(MAT4_IDENTITY, V3F(position.x, position.y, 0.0f));
    call->layer = layer;
    call->color = color;

    return call;
}

draw_call_s* push_draw_call_transformed(draw_group_s* group, texture_s* texture, v2f position, f32 rotation, v2f scale, i32 layer, v4f color) {
    if(group->num_calls + 1 > MAX_DRAW_CALLS) {
        LOG_ERR("reached max draw calls in group\n");
        return NULL;
    }

    group->num_calls ++;
    draw_call_s* call = arena_push(&group->draw_calls, sizeof(draw_call_s));

    call->texture = texture;
    call->position = position;
    call->transformation = glms_translate(MAT4_IDENTITY, V3F(position.x, position.y, 0.0f));
    call->transformation = glms_rotate(call->transformation, rotation, V3F(0.0f, 0.0f, 1.0f));
    call->transformation = glms_scale(call->transformation, V3F(scale.x, scale.y, 1.0f));
    call->layer = layer;
    call->color = color;

    return call;
}

draw_group_s* push_draw_group(renderer_s* renderer, shader_s* shader, camera_s* camera) {
    ASSERT(renderer->num_groups + 1 < MAX_DRAW_GROUPS);
    renderer->num_groups ++;

    draw_group_s* group = arena_push(renderer->groups, sizeof(draw_group_s));

    group->shader = shader;
    group->camera = camera;

    group->framebuffer = renderer->screen_buffer;

    group->mesh = &renderer->unit_mesh;

    group->enable_depth_test = true;
    group->depth_mask = GL_TRUE;
    group->depth_func = GL_LESS;

    group->enable_culling = false;
    group->cull_face = GL_BACK;

    group->projection_type = ORTHOGRAPHIC_PROJECTION;

    group->num_calls = 0;
    usize call_buffer_size = MAX_DRAW_CALLS * sizeof(draw_call_s);
    group->draw_calls = arena_create_in_block(arena_push(&game_state->draw_calls_arena, call_buffer_size), call_buffer_size);

    return group;
}

void render_draw_call(draw_call_s* call, shader_s* shader, mesh_s* mesh, camera_s* camera) {
    if(call->texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, call->texture->handle);
    }

    shader->load_uniforms(call, NULL);

    glBindVertexArray(mesh->vao);
    mesh_enable_attributes(mesh);

    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);

    mesh_disable_attributes(mesh);
    glBindVertexArray(0);
}

void render_draw_group(draw_group_s* group) {
    // update camera matrices
    if(group->projection_type == PERSPECTIVE_PROJECTION)
        group->camera->projection = camera_perspective_projection(group->camera);
    else if(group->projection_type == ORTHOGRAPHIC_PROJECTION)
        group->camera->projection = camera_orthographic_projection(group->camera);

    group->camera->view = camera_view(group->camera);
    group->camera->projection_view = glms_mul(group->camera->projection, group->camera->view);

    glBindFramebuffer(GL_FRAMEBUFFER, group->framebuffer->handle);    
    glDrawBuffers(group->framebuffer->num_textures, group->framebuffer->attachments);

    if(group->enable_depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(group->depth_mask);
        glDepthFunc(group->depth_func);
    }

    if(group->enable_culling) {
        glEnable(GL_CULL_FACE);
        glCullFace(group->cull_face);
    }

    glViewport(0, 0, game_state->window.width, game_state->window.height);

    shader_start(group->shader);

    draw_call_s* calls = group->draw_calls.data;
    for(u32 i = 0; i < group->num_calls; i ++) {
        draw_call_s call = calls[i];
        render_draw_call(&call, group->shader, group->mesh, group->camera);
    }

    // reset state
    shader_stop();

    if(group->enable_depth_test)
        glDisable(GL_DEPTH_TEST);

    if(group->enable_culling)
        glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_draw_groups(renderer_s* renderer) {
    draw_group_s* groups = renderer->groups->data;

    for(u32 i = 0; i < renderer->num_groups; i ++) {
        draw_group_s* group = &groups[i];
        if(group->num_calls == 0) continue;

        render_draw_group(group);
        flush_draw_group(group);
    }
}

void flush_draw_group(draw_group_s* draw_group) {
    arena_clear(&draw_group->draw_calls);
    draw_group->num_calls = 0;
}
