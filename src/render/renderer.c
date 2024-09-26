#include "renderer.h"
#include "game.h"
#include "util/log.h"

void init_renderer(renderer_s* renderer, arena_s* arena) {
    game_state->renderer = (renderer_s) {
        .num_groups= 0,
        .groups = arena
    };
}

draw_call_s* push_draw_call(draw_group_s* group, texture_s* texture, v2f position, i32 layer, v2f uvs, v4f color) {
    if(group->num_calls + 1 > MAX_DRAW_CALLS) {
        LOG_ERR("reached max draw calls in group\n");
        return NULL;
    }

    group->num_calls ++;
    draw_call_s* call = arena_push(&group->draw_calls, sizeof(draw_call_s));

    call->texture = texture;
    call->position = position;
    call->layer = layer;
    call->uvs = uvs;
    call->color = color;

    return call;
}

draw_group_s* push_draw_group(renderer_s* renderer, shader_s* shader, camera_s* camera) {
    ASSERT(renderer->num_groups + 1 < MAX_DRAW_GROUPS);
    renderer->num_groups ++;

    draw_group_s* group = arena_push(renderer->groups, sizeof(draw_group_s));

    group->shader = shader;
    group->camera = camera;

    group->framebuffer = &game_state->screen_buffer;

    group->enable_depth_test = false;
    group->depth_mask = GL_TRUE;
    group->depth_func = GL_LESS;

    group->enable_culling = false;
    group->cull_face = GL_BACK;

    group->num_calls = 0;
    usize call_buffer_size = MAX_DRAW_CALLS * sizeof(draw_call_s);
    group->draw_calls = arena_create_in_block(arena_push(&game_state->draw_calls_arena, call_buffer_size), call_buffer_size);

    return group;
}

void render_draw_call(draw_call_s* call, shader_s* shader, camera_s* camera) {
    if(call->texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, call->texture->id);
    }

    shader->load_uniforms(call);

    glBindVertexArray(game_state->unit_square.vao);
    mesh_enable_attributes(&game_state->unit_square);

    glDrawElements(GL_TRIANGLES, game_state->unit_square.index_count, GL_UNSIGNED_INT, 0);

    mesh_disable_attributes(&game_state->unit_square);
    glBindVertexArray(0);
}

void render_draw_group(draw_group_s* group) {
    // update camera matrices
    group->camera->projection = camera_projection(group->camera);
    group->camera->view = camera_view(group->camera);
    group->camera->projection_view = glms_mul(group->camera->projection, group->camera->view);

    glBindFramebuffer(GL_FRAMEBUFFER, game_state->screen_buffer.id);    
    glDrawBuffers(game_state->screen_buffer.num_textures, game_state->screen_buffer.attachments);

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
    for(u32 j = 0; j < group->num_calls; j ++) {
        draw_call_s call = calls[j];
        render_draw_call(&call, group->shader, group->camera);
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
        draw_group_s group = groups[i];
        render_draw_group(&group);
    }
}
