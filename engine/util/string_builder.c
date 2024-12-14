#include "util.h"

#define INIT_SIZE 8

#define FMT_str  "%s"
#define FMT_char "%c"
#define FMT_i32  "%d"
#define FMT_u32  "%u"
#define FMT_f32  "%f"
#define FMT_bool "%d"
#define FMT_v2f  "%f, %f"
#define FMT_v3f  "%f, %f, %f"
#define FMT_v4f  "%f, %f, %f, %f"

#define MAX_FMT_STR_SIZE 4096

void strb_init(string_builder_s* builder) {
    builder->arena = arena_create_expandable(INIT_SIZE);
    arena_push(&builder->arena, 1);
    memset(builder->arena.data, '\0', 1);

    builder->size = 1;
    builder->capacity = 0;

    builder->str = builder->arena.data;
}

void strb_init_max(string_builder_s* builder, u32 capacity) {
    builder->arena = arena_create(capacity);
    arena_push(&builder->arena, 1);
    memset(builder->arena.data, '\0', 1);

    builder->size = 1;
    builder->capacity = capacity;

    builder->str = builder->arena.data;
}

// TODO(nix3l): im not sure if i should keep it like this or change it
//              its either
//                  => dont copy the input if it doesnt fit
//                  => copy as much of the input as fits
//              currently its the former
// NOTE(nix3l): input size does *NOT* include the null terminator
void strb_catstr(string_builder_s* builder, char* str, u32 len) {
    if(len == 0 || !str) return;
    // if the builder is limited, check if it fits the string first
    if(builder->capacity > 0 && !arena_fits(&builder->arena, len)) return;
    // remove the existing null terminator
    arena_pop(&builder->arena, 1);

    char* slice = arena_push(&builder->arena, len + 1);
    memcpy(slice, str, len);
    slice[len + 1] = '\0';

    builder->size += len;
    builder->str = builder->arena.data;
}

void strb_catc(string_builder_s* builder, char c) {
    // if the builder is limited, check if it fits the character first
    if(builder->capacity > 0 && !arena_fits(&builder->arena, 1)) return;
    // remove the existing null terminator
    arena_pop(&builder->arena, 1);

    char* slice = arena_push(&builder->arena, 2);
    memset(slice, c, 1);
    slice[2] = '\0';

    builder->size ++;
    builder->str = builder->arena.data;
}

void strb_cati32(string_builder_s* builder, i32 val) {
    strb_catf(builder, FMT_i32, val);
}

void strb_catu32(string_builder_s* builder, u32 val) {
    strb_catf(builder, FMT_u32, val);
}

void strb_catf32(string_builder_s* builder, f32 val) {
    strb_catf(builder, FMT_f32, val);
}

void strb_catbool(string_builder_s* builder, bool val) {
    strb_catf(builder, FMT_bool, val ? 1 : 0);
}

void strb_catv2f(string_builder_s* builder, v2f val) {
    strb_catf(builder, FMT_v2f, V2F_EXPAND(val));
}

void strb_catv3f(string_builder_s* builder, v3f val) {
    strb_catf(builder, FMT_v3f, V3F_EXPAND(val));
}

void strb_catv4f(string_builder_s* builder, v4f val) {
    strb_catf(builder, FMT_v4f, V4F_EXPAND(val));
}

void strb_catf(string_builder_s* builder, const char* fmt, ...) {
    i32 rc = 0;
    char buffer[MAX_FMT_STR_SIZE] = {0};
    va_list args;

    va_start(args, fmt);
    rc = vsnprintf(buffer, MAX_FMT_STR_SIZE, fmt, args);
    va_end(args);

    if(rc < 0) {
        LOG_ERR("error writing formatted string to builder\n rc [%d]\n", rc);
        return;
    }

    strb_catstr(builder, buffer, str_len(buffer, MAX_FMT_STR_SIZE));
}

// NOTE(nix3l): does not check if overwriting the null terminator
void strb_setc(string_builder_s* builder, u32 index, char c) {
    if(index > builder->size) return;
    builder->str[index] = c;
}

char* strb_export(string_builder_s* builder, arena_s* arena) {
    if(!arena_fits(arena, builder->size)) {
        LOG_ERR("arena does not fit the string\n");
        return NULL;
    }

    char* output = arena_push(arena, builder->size);
    memcpy(output, builder->str, builder->size);
    return output;
}

void strb_terminate(string_builder_s* builder) {
    arena_free(&builder->arena);
}
