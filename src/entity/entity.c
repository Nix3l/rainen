#include "entity.h"
#include "game.h"

void init_entity_handler(entity_handler_s* handler) {
    handler->lowest_free_id = 0;
    for(u32 i = 0; i < MAX_ENTITIES; i ++) {
        handler->entities[i].id = ENTITY_FREE;
    }
}

entity_s* create_entity(entity_handler_s* handler) {
    entity_s* entity = &handler->entities[handler->lowest_free_id];

    for(u32 i = handler->lowest_free_id + 1; i < MAX_ENTITIES; i ++) {
        if(handler->entities[i].id != ENTITY_FREE) {
            handler->lowest_free_id = i;
            break;
        }
    }

    return entity;
}

void destroy_entity(entity_handler_s* handler, u32 id) {
    handler->entities[id].id = ENTITY_FREE;

    if(handler->lowest_free_id > id) handler->lowest_free_id = id;
}

// TODO(nix3l): redo this, probably remove the whole offset thing
void render_entity(entity_s* entity) {
    push_draw_call_transformed(game_state->default_group,
            entity->sprite.texture,
            glms_vec2_add(entity->position, entity->sprite.offset),
            entity->sprite.rotation,
            entity->sprite.scale,
            entity->sprite.layer,
            entity->sprite.color);
}
