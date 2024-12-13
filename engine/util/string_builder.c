#include "util.h"

#define INIT_SIZE 8

void string_builder_init(string_builder_s* builder) {
    builder->arena = arena_create_expandable(INIT_SIZE);

    builder->size = 0;
    builder->capacity = 0;
    builder->str = builder->arena.data;
}

void string_builder_init_max(string_builder_s* builder, u32 capacity) {
    builder->arena = arena_create(capacity);

    builder->size = 0;
    builder->capacity = capacity;
    builder->str = builder->arena.data;
}

// TODO(nix3l): im not sure if i should keep it like this or change it
//              its either
//                  => dont copy the input if it doesnt fit
//                  => copy as much of the input as fits
//              currently its the former
// NOTE(nix3l): input size does not include the null terminator
void string_builder_concat(string_builder_s* builder, char* str, u32 size) {
    // if the builder is limited, check if it fits the string first
    if(builder->capacity > 0 && !arena_fits(&builder->arena, size)) return;

    // remove the existing null terminator
    arena_pop(&builder->arena, 1);

    // append string and add new null terminator
    char* slice = arena_push(&builder->arena, size + 1);
    memcpy(slice, str, size);
    slice[size + 1] = '\0';

    builder->size += size;
    builder->str = builder->arena.data;
}

// NOTE(nix3l): does not check if overwriting the null terminator
void string_builder_setc(string_builder_s* builder, u32 index, char c) {
    if(index > builder->size) return;
    builder->str[index] = c;
}

char* string_builder_export(string_builder_s* builder, arena_s* arena) {
    if(!arena_fits(arena, builder->size)) {
        LOG_ERR("arena does not fit the string\n");
        return NULL;
    }

    char* output = arena_push(arena, builder->size);
    memcpy(output, builder->str, builder->size);
    return output;
}

void string_builder_terminate(string_builder_s* builder) {
    arena_free(&builder->arena);
}
