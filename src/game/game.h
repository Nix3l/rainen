#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include "base.h"
#include "memory/memory.h"

#include "entity.h"
#include "camera.h"

enum {
    GAME_MAX_ENTITIES = 4096,
};

typedef struct {
    u32 num_dirty_entities;
    pool_t* entity_pool;
    entity_manager_t* render_manager;

    camera_t camera;
} game_ctx_t;

extern game_ctx_t game_ctx;

void game_init();
void game_terminate();
void game_update();

#endif
