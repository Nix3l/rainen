#ifndef SPRITE_H
#define SPRITE_H

#include "base.h"
#include "texture/texture.h"
#include "physics/physics.h"

typedef struct {
    texture_s* texture;

    v2f scale;
    v2f offset;
    f32 rotation;

    i32 layer;

    v4f color;
} sprite_s;

aabb_s sprite_bounding_box(sprite_s* sprite);
void render_sprite(sprite_s* sprite);

#endif
