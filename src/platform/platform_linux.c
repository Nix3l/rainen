#include "memory/memory.h"
#include "platform.h"
#include "util/util.h"

#if OS_LINUX

// gets the size of the entire file in bytes
// returns the file cursor to the start
static u32 file_size(FILE* file) {
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

    u64 size = file_size(file);
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

#endif
