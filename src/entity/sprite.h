#ifndef SPRITE_H
#define SPRITE_H

#include "base.h"
#include "texture/texture.h"

typedef struct {
    texture_s* texture;

    v2f scale;
    v2f offset;
    f32 rotation;

    i32 layer;

    v4f color;
} sprite_s;

void render_sprite(sprite_s* sprite);

#endif
