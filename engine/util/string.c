#include "util.h"

// NOTE(nix3l): taken straight from gcc
u32 str_len(const char* str, u32 max_len) {
    u32 i;
    for(i = 0; i < max_len; ++i)
        if(str[i] == '\0')
            break;

    return i;
}
