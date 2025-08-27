#include "room.h"
#include "physics/bounds.h"
#include "util/math_util.h"

// TILES
v2f tile_get_world_pos(tile_t tile) {
    return v2f_new(
        tile.x * TILE_WIDTH + TILE_WIDTH / 2.0f,
        tile.y * TILE_WIDTH + TILE_WIDTH / 2.0f
    );
}

aabb_t tile_get_aabb(tile_t tile) {
    v2f pos = v2f_new(tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT); 
    return aabb_new(
        pos,
        v2f_new(pos.x + TILE_WIDTH, pos.y + TILE_HEIGHT)
    );
}

// ROOM
room_t room_new() {
    room_t room = {0};

    for(u32 y = 0; y < ROOM_HEIGHT; y ++) {
        for(u32 x = 0; x < ROOM_WIDTH; x ++) {
            room.tiles[y][x] = (tile_t) {
                .x = x,
                .y = y,
            };
        }
    }

    return room;
}

tile_t room_get_tile(room_t* room, u32 x, u32 y) {
    return room->tiles[y][x];
}

void room_set_tile(room_t* room, tile_t tile) {
    room->tiles[tile.y][tile.x] = tile;
}
