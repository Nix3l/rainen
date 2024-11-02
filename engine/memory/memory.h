#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <stdlib.h>
#include "base.h"

// NOTE(nix3l): for now, use the std provided heap allocation functions,
// in case we want to replace with our own allocation interface
// at some point for whatever reason
#define mem_alloc(_bytes) malloc((_bytes))
#define mem_realloc(_buffer, _bytes) realloc((_buffer), (_bytes))
#define mem_free(_ptr) free((_ptr))

typedef struct {
    usize capacity; // amount of data reserved
    usize size; // amount of data committed, serves as the memory position

    void* data;

    bool expandable;
} arena_s;

// create an arena with an immutable fixed size
arena_s arena_create(usize capacity);
// create an arena that expands as its contents do
arena_s arena_create_expandable(usize capacity);

// create a subarena with an immutable fixed size
arena_s subarena_create(arena_s* arena, usize capacity);

// create an arena with an immutable fixed size within a given memory chunk
arena_s arena_create_in_block(void* data, usize capacity);
// create an arena that expands as its contents do within a given memory chunk
arena_s arena_create_expandable_in_block(void* data, usize capacity);

void* arena_push(arena_s* arena, usize bytes);
void arena_pop(arena_s* arena, usize bytes);

// pushes a pointer with as much memory as the arena has left
void* arena_push_to_capacity(arena_s* arena);

// returns whether the given number of bytes can fit in the arena or not
bool arena_fits(arena_s* arena, usize bytes);

// returns the maximum amount of memory that can be pushed on to the arena
usize arena_remaining_capacity(arena_s* arena);

// clear all the used memory in the arena. does not destroy the arena itself
void arena_clear(arena_s* arena);
// frees and destroys the arena. arena can not be used afterwards
void arena_free(arena_s* arena);

typedef enum {
    COMPACT_LIST_EMPTY = 0,
    COMPACT_LIST_TAKEN,
} compact_list_element_t;

// TODO(nix3l): doesnt really handle filling up

typedef struct {
    arena_s* arena; // MUST be zero'd

    u32 element_size; // size of each stored element
    u32 capacity; // max amount of stored elements

    u32 count; // amount of stored elements
               // does NOT equal the index of the last element
    u32 first_free_index; // index of first free element
    void* contents; // data
    compact_list_element_t* elements; // empty slot or taken slot
} compact_list_s;

compact_list_s create_compact_list(arena_s* arena, u32 element_size, u32 capacity);

void* compact_list_push(compact_list_s* list, u32* index);
void* compact_list_get(compact_list_s* list, u32 index);
void compact_list_remove(compact_list_s* list, u32 index);

#endif
