#include "base_ctx.h"

#if OS_LINUX

// needed to use posix functions (clock_gettime)
#define _POSIX_C_SOURCE 199309L
#include "platform.h"
#include "util/util.h"
#include <time.h>

// gets the size of the entire file in bytes
// returns the file cursor to the start
static u32 file_get_size(FILE* file) {
    fseek(file, 0L, SEEK_END);
    u32 size = ftell(file);
    rewind(file);
    return size;
}

DEVONLY range_t platform_load_file(arena_t* arena, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if(!file) {
        LOG_ERR("couldnt open file [%s] for read\n", filename);
        return RANGE_EMPTY;
    }

    u64 size = file_get_size(file);
    if(arena->type == EXPAND_TYPE_IMMUTABLE && !arena_fits(arena, size + 1)) {
        LOG_ERR("cant fit entire file into arena\n");
        fclose(file);
        return RANGE_EMPTY;
    } else {
        arena_prepare(arena, size + 1);
    }

    char* output = arena_push(arena, size + 1);
    usize read_length = fread(output, 1, size, file);

    // even if the whole file wasnt read,
    // at least return the stuff we were able to read
    // but leave a warning
    if(read_length != size) LOG_WARN("read error occured on file [%s]\n", filename);

    output[read_length + 1] = '\0';
    fclose(file);

    return (range_t) {
        .size = read_length + 1,
        .ptr = output,
    };
}

u64 platform_get_ticks() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

f32 platform_get_milli_diff(u64 ticks) {
    u64 curr = platform_get_ticks();
    return (f32) ((curr - ticks) / 1000000.0f);
}

f32 platform_get_milli_diff_reset(u64* ticks) {
    u64 curr = platform_get_ticks();
    f32 dt = (f32) ((curr - *ticks) / 1000000.0f);
    *ticks = curr;
    return dt;
}

f32 platform_ticks_to_milli(u64 t) {
    return (f32) (t / 1000000.0f);
}

f32 platform_ticks_to_sec(u64 t) {
    return (f32) (t / 1000000000.0f);
}

#endif
