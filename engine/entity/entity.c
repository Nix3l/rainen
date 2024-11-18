#include "entity.h"

#include "engine.h"

void init_entity_handler(entity_handler_s* handler, arena_s* entities_arena, u32 entities_capacity) {
    handler->entities_arena = entities_arena;
    handler->entities = create_compact_list(entities_arena, sizeof(entity_s), entities_capacity);
}

entity_s* create_entity(entity_handler_s* handler) {
    u32 slot_taken;
    entity_s* entity = compact_list_push(&handler->entities, &slot_taken);
    entity->handle = slot_taken;
    entity->flags = ENTITY_NONE;
    return entity;
}

void destroy_entity(entity_handler_s* handler, u32 handle) {
    compact_list_remove(&handler->entities, handle);
}

entity_s* entity_data(u32 handle) {
    return compact_list_get(&engine_state->entity_handler.entities, handle);
}

// TODO(nix3l): redo this, probably remove the whole offset thing
void render_entity(entity_s* entity) {
    push_draw_call(engine_state->default_group,
            NULL,
            entity->sprite.texture,
            V2F_ADD(entity->position, entity->sprite.offset),
            entity->sprite.rotation,
            entity->sprite.scale,
            0.0f,
            entity->sprite.layer,
            entity->sprite.color);
}
