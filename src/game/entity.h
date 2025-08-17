#ifndef _ENTITY_H
#define _ENTITY_H

#include "base.h"
#include "gfx/gfx.h"

#define ENT_INVALID_ID (0)

typedef struct { handle_t id; } entity_t;

typedef enum entity_tags_t {
    ENT_TAGS_NONE = 0x00,
    ENT_TAGS_RENDER = 0x01,
} entity_tags_t;

// ENTITY DATA
typedef struct material_t {
    v4f colour;
} material_t;

typedef struct transform_t {
    v2f position;
    f32 rotation;
    v2f size; // actual size of the entity
    v2f scale; // scalar factor for each axis
    i32 z;
} transform_t;

typedef struct entity_data_t {
    entity_tags_t tags;
    transform_t transform;
    material_t material;
} entity_data_t;

typedef struct entity_info_t {
    entity_tags_t tags;
    transform_t transform;
    material_t material;
} entity_info_t;

typedef enum entity_slot_state_t {
    ENT_STATE_FREE = 0,
    ENT_STATE_DIRTY,
    ENT_STATE_ACTIVE,
} entity_slot_state_t;

typedef struct entity_slot_t {
    entity_slot_state_t state;
    entity_data_t data;
} entity_slot_t;

entity_t entity_new(entity_info_t info);
void entity_destroy(entity_t ent);

entity_slot_t* entity_get_slot(entity_t ent);
entity_slot_state_t entity_get_state(entity_t ent);
entity_data_t* entity_get_data(entity_t ent);

void entity_mark_dirty(entity_t ent);

typedef struct entity_manager_t {
    const char* label;
    vector_t batch;
} entity_manager_t;

void entity_manager_push(entity_manager_t* manager, entity_t ent);

// TODO(nix3l): dirty entity garbage collection

void entity_update();

#endif
