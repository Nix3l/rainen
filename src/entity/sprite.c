#include "sprite.h"
#include "game.h"

void render_sprite(sprite_s* sprite) {
    push_draw_call(game_state->default_group,
            NULL,
            sprite->texture,
            sprite->offset,
            sprite->rotation,
            sprite->scale,
            sprite->layer,
            sprite->color);
}
