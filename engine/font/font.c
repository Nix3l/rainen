#include "font.h"

#include "util/log.h"
#include "platform/platform.h"

// TODO(nix3l): get glyph information from font

// probably big enough
#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT 1024

// NOTE(nix3l): should probably improve this later, not very usable
//              maybe add options for which font sizes to pack?
void init_font(font_s* font, char* filename, arena_s* arena) {
    void* font_data = DEBUGplatform_load_file(filename, NULL, arena);

    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, font_data, stbtt_GetFontOffsetForIndex(font_data, 0));

    font->info = font_info;

    stbtt_pack_context pack_context;
    void* packed_atlas = arena_push(arena, ATLAS_WIDTH * ATLAS_HEIGHT);

    // data for each pack of font ranges
    // 95 chars for the standard ascii set (from 32-127)
    stbtt_packedchar packed_chars[NUM_FONT_SIZES][95];
    // pack a range for each desired font size
    stbtt_pack_range ranges[NUM_FONT_SIZES] = {
        { 12.0f, 32, NULL, 95, packed_chars[0 ], 0, 0, },
        { 16.0f, 32, NULL, 95, packed_chars[1 ], 0, 0, },
        { 20.0f, 32, NULL, 95, packed_chars[2 ], 0, 0, },
        { 24.0f, 32, NULL, 95, packed_chars[3 ], 0, 0, },
        { 28.0f, 32, NULL, 95, packed_chars[4 ], 0, 0, },
        { 32.0f, 32, NULL, 95, packed_chars[5 ], 0, 0, },
        { 36.0f, 32, NULL, 95, packed_chars[6 ], 0, 0, },
        { 40.0f, 32, NULL, 95, packed_chars[7 ], 0, 0, },
        { 44.0f, 32, NULL, 95, packed_chars[8 ], 0, 0, },
        { 48.0f, 32, NULL, 95, packed_chars[9 ], 0, 0, },
        { 52.0f, 32, NULL, 95, packed_chars[10], 0, 0, },
        { 56.0f, 32, NULL, 95, packed_chars[11], 0, 0, },
        { 60.0f, 32, NULL, 95, packed_chars[12], 0, 0, },
        { 64.0f, 32, NULL, 95, packed_chars[13], 0, 0, },
        { 68.0f, 32, NULL, 95, packed_chars[14], 0, 0, },
        { 72.0f, 32, NULL, 95, packed_chars[15], 0, 0, },
    };

    stbtt_PackBegin(&pack_context, packed_atlas, ATLAS_WIDTH, ATLAS_HEIGHT, 0, 1, NULL);

    // currently no oversampling
    // TODO(nix3l): what is oversampling
    stbtt_PackSetOversampling(&pack_context, 1, 1);

    stbtt_PackFontRanges(&pack_context, font_data, 0, ranges, ARRAY_SIZE(ranges));
    stbtt_PackEnd(&pack_context);

    font->atlas = create_texture(ATLAS_WIDTH, ATLAS_HEIGHT, TEXTURE_R, TEXTURE_8b, packed_atlas);
    memcpy(font->packed_chars, packed_chars, sizeof(stbtt_packedchar) * NUM_FONT_SIZES * 95);
}
