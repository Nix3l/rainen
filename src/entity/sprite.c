#include "sprite.h"
#include "game.h"

void render_sprite(sprite_s* sprite) {
    push_draw_call_transformed(game_state->default_group,
            sprite->texture,
            sprite->offset,
            sprite->rotation,
            sprite->scale,
            sprite->layer,
            sprite->color);
}
