#ifndef ENTITY_H
#define ENTITY_H

#include "base.h"
#include "sprite.h"

#define MAX_ENTITIES (4096)
#define ENTITY_FREE ((u32)(-1))

typedef struct {
    u32 id;

    v2f position;
    sprite_s sprite;
} entity_s;

typedef struct {
    entity_s entities[MAX_ENTITIES];

    u32 lowest_free_id;
} entity_handler_s;

void init_entity_handler(entity_handler_s* entity_handler);

entity_s* create_entity(entity_handler_s* handler);
void destroy_entity(entity_handler_s* handler, u32 id);

void render_entity(entity_s* entity);

#endif
