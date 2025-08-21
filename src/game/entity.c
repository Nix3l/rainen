#include "entity.h"
#include "game.h"
#include "errors/errors.h"
#include "memory/memory.h"
#include "render/render.h"
#include <string.h>

entity_t entity_new(entity_info_t info) {
    entity_t ent = {0};
    entity_slot_t* slot = pool_push(game_ctx.entity_pool, &ent.id);
    mem_clear(slot, sizeof(entity_slot_t));

    if(info.transform.scale.x == 0.0f) info.transform.scale.x = 1.0f;
    if(info.transform.scale.y == 0.0f) info.transform.scale.y = 1.0f;

    slot->data.tags = info.tags;
    slot->data.transform = info.transform;
    slot->data.material = info.material;

    if(info.tags & ENT_TAGS_RENDER) entity_manager_push(game_ctx.render_manager, ent);

    return ent;
}

void entity_destroy(entity_t ent) {
    if(ent.id == ENT_INVALID_ID) {
        LOG_ERR_CODE(ERR_ENT_BAD_ID);
        return;
    }

    pool_free(game_ctx.entity_pool, ent.id);
}

entity_slot_t* entity_get_slot(entity_t ent) {
    if(ent.id == ENT_INVALID_ID) {
        LOG_ERR_CODE(ERR_ENT_BAD_ID);
        return NULL;
    }

    return pool_get(game_ctx.entity_pool, ent.id);
}

// TODO(nix3l): huh??
entity_slot_state_t entity_get_state(entity_t ent) {
    entity_slot_t* slot = pool_get(game_ctx.entity_pool, ent.id);
    if(!slot) {
        LOG_ERR_CODE(ERR_ENT_BAD_SLOT);
        return ENT_STATE_FREE;
    }

    return slot->state;
}

entity_data_t* entity_get_data(entity_t ent) {
    entity_slot_t* slot = pool_get(game_ctx.entity_pool, ent.id);
    if(!slot) {
        LOG_ERR_CODE(ERR_ENT_BAD_SLOT);
        return NULL;
    }

    return &slot->data;
}

void entity_mark_dirty(entity_t ent) {
    entity_slot_t* slot = pool_get(game_ctx.entity_pool, ent.id);
    if(!slot) {
        LOG_ERR_CODE(ERR_ENT_BAD_SLOT);
        return;
    }

    slot->state = ENT_STATE_DIRTY;
    game_ctx.num_dirty_entities ++;
}

void entity_manager_push(entity_manager_t* manager, entity_t ent) {
    if(ent.id == ENT_INVALID_ID) {
        LOG_ERR_CODE(ERR_ENT_BAD_ID);
        return;
    }

    vector_push_data(&manager->batch, &ent);
}

// RENDER MANAGER UPDATE
static v3f entity_compute_draw_scale(transform_t transform) {
    return v3f_new(transform.size.x * transform.scale.x, transform.size.y * transform.scale.y, 1.0f);
}

static void entity_render_update(entity_t ent) {
    entity_data_t* data = entity_get_data(ent);
    if(!data) {
        LOG_ERR_CODE(ERR_ENT_BAD_SLOT);
        return;
    }

    render_push_draw_call(
        &render_ctx.renderer,
        (draw_call_t) {
            .position = v3f_new(data->transform.position.x, data->transform.position.y, data->transform.z),
            .rotation = v3f_new(0.0f, 0.0f, data->transform.rotation),
            .scale = entity_compute_draw_scale(data->transform),
            .colour = data->material.colour,
        }
    );
}

// UPDATE
#define entity_manager_update(_manager, _update) do {           \
    for(u32 i = 0; i < game_ctx._manager->batch.size; i ++) {   \
        entity_t ent;                                           \
        vector_fetch(&game_ctx._manager->batch, i, &ent);       \
        entity_slot_t* slot = entity_get_slot(ent);             \
        if(!slot) {                                             \
            LOG_ERR_CODE(ERR_ENT_BAD_SLOT);                     \
            return;                                             \
        }                                                       \
                                                                \
        _update(ent);                                           \
    }                                                           \
} while(0)

static void entity_manager_collect_garbage(entity_manager_t* manager) {
    if(!manager) {
        LOG_ERR_CODE(ERR_ENT_BAD_MANAGER);
        return;
    }

    for(u32 i = manager->batch.size - 1; i > 0; i --) {
        entity_t ent;
        vector_fetch(&manager->batch, i, &ent);

        bool garbage = false;

        entity_slot_t* slot = entity_get_slot(ent);
        if(!slot) garbage = true;
        else if(slot->state == ENT_STATE_DIRTY) garbage = true;

        if(garbage) vector_remove(&manager->batch, i);
    }
}

static void entity_collect_garbage() {
    entity_manager_collect_garbage(game_ctx.render_manager);

    // not technically the most efficient method
    // but it works, and its fast, and it wont exactly get slow
    // unless there are way more entities than this engine supports
    // so no need to change this code yet
    u32 num_destroyed = 0;
    pool_iter_t iter = { .absolute_index = 1 };
    while(pool_iter(game_ctx.entity_pool, &iter)) {
        entity_slot_t* slot = iter.data;
        if(slot->state == ENT_STATE_DIRTY) {
            entity_destroy((entity_t){iter.handle});
            num_destroyed ++;
        }
    }

    if(game_ctx.num_dirty_entities != num_destroyed) LOG_ERR_CODE(ERR_ENT_GARBAGE_COLLECTION_MISMATCH);
    game_ctx.num_dirty_entities -= num_destroyed;
}

void entity_update() {
    entity_manager_update(render_manager, entity_render_update);
    if(game_ctx.num_dirty_entities > 0) entity_collect_garbage();
}
