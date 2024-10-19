#ifndef FONT_H
#define FONT_H

#include "base.h"
#include "memory/memory.h"
#include "texture/texture.h"

// NOTE(nix3l): recommended by nothings for better bitmap packing
#include "stb_rect_pack.h"
#include "stb_truetype.h"

#define NUM_FONT_SIZES 16

typedef struct {
    texture_s atlas;

    stbtt_fontinfo info;
    stbtt_packedchar packed_chars[NUM_FONT_SIZES][95];
} font_s;

void init_font(font_s* font, char* filename, arena_s* arena);

#endif
