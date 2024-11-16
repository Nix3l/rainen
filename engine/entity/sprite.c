#include "sprite.h"

#include "engine.h"

aabb_s sprite_bounding_box(sprite_s* sprite) {
    return aabb_create(sprite->texture->width * sprite->scale.x, sprite->texture->height * sprite->scale.y);
}

void render_sprite(sprite_s* sprite) {
    push_draw_call(engine_state->default_group,
            NULL,
            sprite->texture,
            sprite->offset,
            sprite->rotation,
            sprite->scale,
            sprite->layer,
            sprite->color);
}
