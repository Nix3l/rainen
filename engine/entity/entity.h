#ifndef ENTITY_H
#define ENTITY_H

#include "base.h"
#include "sprite.h"
#include "physics/physics.h"

// NOTE(nix3l): currently, entities have a maximum amount that can exist at once
//              if i ever for whatever reason decide i want to have an unlimited amount,
//              change up the code so that entities can only be referred to by their handle
//              since every time the arena expands or cuts down, the pointers will all change

// NOTE(nix3l): *ONLY* refer to entites by their handles!!
//              if entity data needed, just look it up

typedef enum {
    ENTITY_NONE = 0x00,
    ENTITY_HAS_PHYSICS = 0x01,
} entity_flags_t;

// TODO(nix3l): entity tagging system to manage behaviour
//              i.e. have an array of tags for each entity
//              and give it the behaviour data handle
//              for each added tag
typedef struct {
    u32 handle; // index of entity in entities arena
    u32 flags;

    v2f position;
    sprite_s sprite;

    u32 rigidbody;
} entity_s;

typedef struct {
    arena_s* entities_arena;
    compact_list_s entities;
} entity_handler_s;

void init_entity_handler(entity_handler_s* entity_handler, arena_s* entities_arena, u32 entities_capacity);

entity_s* create_entity(entity_handler_s* handler);
void destroy_entity(entity_handler_s* handler, u32 handle);

entity_s* entity_data(u32 handle);

void render_entity(entity_s* entity);

#endif
