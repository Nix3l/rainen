#ifndef _ROOM_H
#define _ROOM_H

#include "base.h"
#include "memory/memory.h"
#include "gfx/gfx.h"
#include "physics/aabb.h"

enum {
    TILE_WIDTH = 24,
    TILE_HEIGHT = 24,
    ROOM_WIDTH = 256,
    ROOM_HEIGHT = 64,
};

typedef enum tile_tags_t {
    TILE_TAGS_NONE   = 0x00,
    TILE_TAGS_RENDER = 0x01,
    TILE_TAGS_SOLID  = 0x02,
} tile_tags_t;

typedef struct tile_t {
    tile_tags_t tags;
    u32 x, y;
    v4f col;
} tile_t;

typedef struct room_t {
    tile_t tiles[ROOM_WIDTH][ROOM_HEIGHT];
    aabb_t volume;
} room_t;

tile_t room_get_tile(room_t* room, u32 x, u32 y);
void room_set_tile(room_t* room, tile_t tile);

#endif
