#include "room.h"
#include "physics/bounds.h"
#include "render/render.h"
#include "io/io.h"
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

// CAMERA VOLUMES
aabb_t camera_volume_bounds(camera_volume_t vol) {
    return aabb_new_rect(vol.center, v2f_scale(vol.dimensions, 0.5f));
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

    room.camvol = (camera_volume_t) {
        .center = v2f_new(TILE_WIDTH * ROOM_WIDTH / 2.0f, TILE_HEIGHT * ROOM_HEIGHT / 2.0f),
        .dimensions = v2f_new(io_ctx.window.width, io_ctx.window.height),
    };

    return room;
}

tile_t room_get_tile(room_t* room, u32 x, u32 y) {
    return room->tiles[y][x];
}

void room_set_tile(room_t* room, tile_t tile) {
    room->tiles[tile.y][tile.x] = tile;
}

void room_render(room_t* room, draw_group_t* group) {
    v3f scale = v3f_new(TILE_WIDTH / 2.0f, TILE_HEIGHT / 2.0f, 1.0f);
    for(u32 y = 0; y < ROOM_HEIGHT; y ++) {
        for(u32 x = 0; x < ROOM_WIDTH; x ++) {
            tile_t tile = room_get_tile(room, x, y);
            v2f pos = tile_get_world_pos(tile);

            if(!(tile.tags & TILE_TAGS_RENDER)) continue;
            render_push_draw_call(group, (draw_call_t) {
                .position = v3f_new(pos.x, pos.y, 0),
                .scale = scale,
                .colour = tile.data.col,
            });
        }
    }
}
