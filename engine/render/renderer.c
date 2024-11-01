#include "renderer.h"

#include "engine.h"

#include "util/log.h"
#include "util/math.h"

void init_renderer(renderer_s* renderer, arena_s* arena, fbo_s* screen) {
    *renderer = (renderer_s) {
        .num_groups = 0,
        .groups = arena,

        .screen_buffer = screen,
        .unit_mesh = primitive_unit_square(),
    };
}

draw_group_s* push_draw_group(renderer_s* renderer, shader_s* shader, camera_s* camera) {
    ASSERT(renderer->num_groups + 1 < MAX_DRAW_GROUPS);
    renderer->num_groups ++;

    draw_group_s* group = arena_push(renderer->groups, sizeof(draw_group_s));

    group->shader = shader;
    group->camera = camera;

    group->framebuffer = renderer->screen_buffer;

    group->fallback_mesh = &renderer->unit_mesh;

    group->enable_depth_test = true;
    group->depth_mask = GL_TRUE;
    group->depth_func = GL_LESS;

    group->enable_culling = false;
    group->cull_face = GL_BACK;

    group->projection_type = ORTHOGRAPHIC_PROJECTION;

    group->num_calls = 0;
    usize call_buffer_size = MAX_DRAW_CALLS * sizeof(draw_call_s);
    group->draw_calls = arena_create_in_block(arena_push(&engine_state->draw_calls_arena, call_buffer_size), call_buffer_size);

    return group;
}

draw_call_s* push_draw_call(draw_group_s* group, mesh_s* mesh, texture_s* texture, v2f position, f32 rotation, v2f scale, i32 layer, v4f color) {
    if(group->num_calls + 1 > MAX_DRAW_CALLS) {
        LOG_ERR("reached max draw calls in group\n");
        return NULL;
    }

    group->num_calls ++;
    draw_call_s* call = arena_push(&group->draw_calls, sizeof(draw_call_s));

    call->camera = group->camera;
    call->mesh = mesh;
    call->texture = texture;
    call->position = position;
    call->transformation = get_transformation_matrix(position, rotation, scale);
    call->layer = layer;
    call->color = color;

    return call;
}

// TODO(nix3l): have this be a separate thing somewhere
//              that just returns a mesh, so we dont waste
//              time and resources making and destroying the same mesh every frame
// TODO(nix3l): this sucks
// => handle line breaks and word wrap
// => text aligning?
// => width and height constraints
// => italics, maybe bold (underline would be a pain and not worth it)
draw_call_s* push_text_draw_call(draw_group_s* group, font_s* font, i32 size, char* text, u32 text_length, v2f start, arena_s* arena) {
    if(group->num_calls + 1 > MAX_DRAW_CALLS) {
        LOG_ERR("reached max draw calls in group\n");
        return NULL;
    }

    u32 vertex_count = text_length * 6; // 6 vertices per glyph (no indices, 2 triangles)
    f32* vertices = arena_push(arena, sizeof(f32) * vertex_count * 3);
    f32* uvs = arena_push(arena, sizeof(f32) * vertex_count * 2);

    u32 curr_vertex = 0;
    u32 curr_uvs = 0;

    v2f pos = start;
    for(u32 i = 0; i < text_length; i ++) {
        char glyph = text[i];
        stbtt_aligned_quad quad;

        stbtt_GetPackedQuad(font->packed_chars[size],
                font->atlas.width, font->atlas.height,
                glyph - ' ', // dont question the ascii
                &pos.x, &pos.y,
                &quad,
                0);

        // stbtt assumes y-axis going down, so have to flip
        quad.y0 = -quad.y0;
        quad.y1 = -quad.y1;

        // first triangle
        vertices[curr_vertex++] = quad.x0; // bottom left
        vertices[curr_vertex++] = quad.y1;
        vertices[curr_vertex++] = 0.0f;
        vertices[curr_vertex++] = quad.x1; // top right
        vertices[curr_vertex++] = quad.y0;
        vertices[curr_vertex++] = 0.0f;
        vertices[curr_vertex++] = quad.x0; // top left
        vertices[curr_vertex++] = quad.y0;
        vertices[curr_vertex++] = 0.0f;

        uvs[curr_uvs++] = quad.s0; // bottom left 
        uvs[curr_uvs++] = quad.t1;
        uvs[curr_uvs++] = quad.s1; // top right
        uvs[curr_uvs++] = quad.t0;
        uvs[curr_uvs++] = quad.s0; // top left
        uvs[curr_uvs++] = quad.t0;

        // second triangle
        vertices[curr_vertex++] = quad.x0; // bottom left
        vertices[curr_vertex++] = quad.y1;
        vertices[curr_vertex++] = 0.0f;
        vertices[curr_vertex++] = quad.x1; // bottom right
        vertices[curr_vertex++] = quad.y1;
        vertices[curr_vertex++] = 0.0f;
        vertices[curr_vertex++] = quad.x1; // top right
        vertices[curr_vertex++] = quad.y0;
        vertices[curr_vertex++] = 0.0f;

        uvs[curr_uvs++] = quad.s0; // bottom left 
        uvs[curr_uvs++] = quad.t1;
        uvs[curr_uvs++] = quad.s1; // bottom right
        uvs[curr_uvs++] = quad.t1;
        uvs[curr_uvs++] = quad.s1; // top right 
        uvs[curr_uvs++] = quad.t0;
    }

    // TODO(nix3l): mark mesh for deletion after call
    mesh_s result = create_mesh_arrays(vertices, uvs, NULL, NULL, vertex_count);

    arena_pop(arena, sizeof(f32) * vertex_count * 2); // uvs
    arena_pop(arena, sizeof(f32) * vertex_count * 3); // vertices

    mesh_s* mesh = arena_push(arena, sizeof(mesh_s));
    memcpy(mesh, &result, sizeof(mesh_s));

    group->num_calls ++;
    draw_call_s* call = arena_push(&group->draw_calls, sizeof(draw_call_s));

    call->camera = group->camera;
    call->mesh = mesh;
    call->texture = &font->atlas;
    call->position = start;
    call->transformation = MAT4_IDENTITY;
    call->layer = 0;
    call->color = V4F_ONE();

    return call;
}

void render_draw_call(draw_call_s* call, shader_s* shader) {
    if(call->texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, call->texture->handle);
    }

    shader->load_uniforms(call, NULL);

    glBindVertexArray(call->mesh->vao);
    mesh_enable_attributes(call->mesh);

    if(call->mesh->data & MESH_INDICES)
        glDrawElements(GL_TRIANGLES, call->mesh->index_count, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(GL_TRIANGLES, 0, call->mesh->vertex_count);

    mesh_disable_attributes(call->mesh);
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

    glViewport(0, 0, engine_state->window.width, engine_state->window.height);

    shader_start(group->shader);

    draw_call_s* calls = group->draw_calls.data;
    for(u32 i = 0; i < group->num_calls; i ++) {
        draw_call_s call = calls[i];
        if(!call.mesh && group->fallback_mesh) call.mesh = group->fallback_mesh;

        render_draw_call(&call, group->shader);
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
