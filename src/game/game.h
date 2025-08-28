#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include "base.h"
#include "memory/memory.h"

#include "entity.h"
#include "camera.h"
#include "room.h"

typedef struct {
    entity_t player;

    camera_t camera;

    renderer_t renderer;
    room_t room;
} game_ctx_t;

extern game_ctx_t game_ctx;

void game_init();
void game_terminate();

void game_update();
void game_render();

void game_load_room(room_t room);

#endif
