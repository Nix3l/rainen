#include "entity.h"

#include "engine.h"

void init_entity_handler(entity_handler_s* handler, arena_s* entities_arena, u32 entities_capacity) {
    handler->entities_arena = entities_arena;
    handler->entities_capacity = entities_capacity;

    handler->entity_count = 0;
    handler->first_free_entity = 0;
    handler->entities = arena_push(handler->entities_arena, entities_capacity * sizeof(entity_s));
}

entity_s* create_entity(entity_handler_s* handler) {
    entity_s* entity = &handler->entities[handler->first_free_entity];
    entity->state = ENTITY_ACTIVE;
    entity->flags = ENTITY_NONE;

    for(u32 i = handler->first_free_entity + 1; i < handler->entities_capacity; i ++) {
        if(handler->entities[i].state == ENTITY_EMPTY) {
            handler->first_free_entity = i;
            break;
        }
    }

    handler->entity_count ++;

    return entity;
}

void destroy_entity(entity_handler_s* handler, u32 handle) {
    handler->entities[handle].state = ENTITY_EMPTY;
    handler->entities[handle].flags = ENTITY_NONE;

    if(handle < handler->first_free_entity)
        handler->first_free_entity = handle;

    handler->entity_count --;
}

entity_s* entity_data(u32 handle) {
    return &engine_state->entity_handler.entities[handle];
}

// TODO(nix3l): redo this, probably remove the whole offset thing
void render_entity(entity_s* entity) {
    push_draw_call(engine_state->default_group,
            NULL,
            entity->sprite.texture,
            V2F_ADD(entity->position, entity->sprite.offset),
            entity->sprite.rotation,
            entity->sprite.scale,
            entity->sprite.layer,
            entity->sprite.color);
}
