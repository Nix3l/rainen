#include "memory.h"

#define ARENA_GROWTH_FACTOR (1.5f)

arena_s arena_create(usize capacity) {
    return (arena_s) {
        .capacity = capacity,
        .size = 0,
        .data = mem_alloc(sizeof(u8) * capacity),
        .expandable = false
    };
}

arena_s arena_create_expandable(usize capacity) {
    return (arena_s) {
        .capacity = capacity,
        .size = 0,
        .data = mem_alloc(sizeof(u8) * capacity),
        .expandable = true 
    };
}

arena_s arena_create_in_block(void* data, usize capacity) {
    ASSERT(data);
    return (arena_s) {
        .capacity = capacity,
        .size = 0,
        .data = data,
        .expandable = false
    };
}

arena_s arena_create_expandable_in_block(void* data, usize capacity) {
    ASSERT(data);
    return (arena_s) {
        .capacity = capacity,
        .size = 0,
        .data = data,
        .expandable = true 
    };
}


// NOTE(nix3l): growing and shrinking uses the same logic as vectors, check vectors.c for more info
static void grow_arena(arena_s* arena) {
    arena->capacity *= ARENA_GROWTH_FACTOR;
    arena->data = mem_realloc(arena->data, arena->capacity);
}

static void shrink_arena(arena_s* arena) {
    arena->capacity /= ARENA_GROWTH_FACTOR;
    arena->data = mem_realloc(arena->data, arena->capacity);
}

void* arena_push(arena_s* arena, usize size) {
    if(arena->capacity < arena->size + size) {
        if(arena->expandable) grow_arena(arena);
        else ASSERT_BREAK(arena->capacity > arena->size + size);
    }

    void* data = &arena->data[arena->size];
    arena->size += size;

    return data;
}

void arena_pop(arena_s* arena, usize bytes) {
    ASSERT(arena->size >= bytes);

    if(arena->expandable && arena->size - bytes < arena->capacity * ARENA_GROWTH_FACTOR)
        shrink_arena(arena);
    
    arena->size -= bytes;
}

void* arena_push_to_capacity(arena_s* arena) {
    return arena_push(arena, arena->capacity - arena->size);
}

bool arena_fits(arena_s* arena, usize bytes) {
    return arena->size + bytes <= arena->capacity && !arena->expandable;
}

usize arena_remaining_capacity(arena_s* arena) {
    return arena->capacity - arena->size;
}

void arena_clear(arena_s* arena) {
    arena->size = 0;
    // NOTE(nix3l): no real need to zero the memory here, arena->size = 0 is enough
    // makes it way too slow when clearing large arenas
    // MEM_ZERO(arena->data, arena->capacity);
}

void arena_free(arena_s* arena) {
    arena->capacity = 0;
    arena->size = 0;
    mem_free(arena->data);
    arena->data = NULL;
}
