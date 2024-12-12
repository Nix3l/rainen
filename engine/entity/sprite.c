#include "sprite.h"

#include "engine.h"

aabb_s sprite_bounding_box(sprite_s* sprite) {
    return aabb_create_dimensions(sprite->texture->width * sprite->scale.x, sprite->texture->height * sprite->scale.y);
}

void render_sprite(sprite_s* sprite) {
    push_draw_call(engine->default_group,
            NULL,
            sprite->texture,
            sprite->offset,
            sprite->rotation,
            sprite->scale,
            0.0f,
            sprite->layer,
            sprite->color);
}
