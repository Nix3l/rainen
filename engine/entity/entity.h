#ifndef ENTITY_H
#define ENTITY_H

#include "base.h"
#include "sprite.h"

// NOTE(nix3l): currently, entities have a maximum amount that can exist at once
//              if i ever for whatever reason decide i want to have an unlimited amount,
//              change up the code so that entities can only be referred to by their handle
//              since every time the arena expands or cuts down, the pointers will all change

typedef enum {
    ENTITY_NONE = 0x0,
} entity_flags_t;

typedef enum {
    ENTITY_EMPTY = 0,
    ENTITY_ACTIVE,
    ENTITY_DISABLED,
} entity_state_t;

typedef struct {
    u32 handle;
    u32 flags;
    u32 state;

    v2f position;
    sprite_s sprite;
} entity_s;

typedef struct {
    arena_s* entities_arena;

    u32 entities_capacity;

    u32 entity_count;
    u32 first_free_entity;
    entity_s* entities;
} entity_handler_s;

void init_entity_handler(entity_handler_s* entity_handler, arena_s* entities_arena, u32 entities_capacity);

entity_s* create_entity(entity_handler_s* handler);
void destroy_entity(entity_handler_s* handler, u32 handle);

void render_entity(entity_s* entity);

#endif
