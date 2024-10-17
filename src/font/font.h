#ifndef FONT_H
#define FONT_H

#include "base.h"
#include "memory/memory.h"
#include "texture/texture.h"

typedef struct {
    texture_s atlas;
} font_s;

void init_font(font_s* font, char* filename, arena_s* arena);

#endif
